#include "../include/FileManager.hpp"

void FileManager::init() { 
    isLoaded = false; 
    entries.clear();
    
    // Load Folder Icon
    Image img = LoadImage("assets/folder.png");
    if (img.data != NULL) {
        ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
        ImageResizeNN(&img, Config::ICON_SIZE_SMALL, Config::ICON_SIZE_SMALL); 
        folderIcon = LoadTextureFromImage(img);
        SetTextureFilter(folderIcon, TEXTURE_FILTER_BILINEAR);
        UnloadImage(img);
    }
}

void FileManager::cleanup() {
    if (folderIcon.id > 0) UnloadTexture(folderIcon);
}

void FileManager::openFolderDialog() {
    std::string path = OpenWindowsFolderPicker();
    if (!path.empty()) { 
        currentPath = path; 
        isLoaded = true; 
        scrollIndex = 0; 
        refresh(); 
    }
}

void FileManager::openFileDialog() {
    std::string path = OpenWindowsFilePicker();
    if (!path.empty()) selectedFile = path;
}

void FileManager::refresh() {
    if (!isLoaded) return;
    entries.clear();
    try {
        for (const auto& entry : fs::directory_iterator(currentPath)) entries.push_back(entry);
        std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
            if (a.is_directory() != b.is_directory()) return a.is_directory();
            return a.path().filename() < b.path().filename();
        });
    } catch (...) { isLoaded = false; }
}

std::string FileManager::popSelectedFile() {
    std::string s = selectedFile; selectedFile = ""; return s;
}

void FileManager::update(Rectangle bounds, bool isFocused) {
    // Offset for Header Height (25px)
    float headerH = 25.0f;
    Rectangle contentBounds = {bounds.x, bounds.y + headerH, bounds.width, bounds.height - headerH};

    if (isLoaded) {
        if (isFocused) {
            float wheel = GetMouseWheelMove();
            scrollIndex -= (int)wheel; if (scrollIndex < 0) scrollIndex = 0;
        }
        if (isFocused && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 m = GetMousePosition();
            if (CheckCollisionPointRec(m, contentBounds)) {
                int idx = (int)((m.y - contentBounds.y) / itemHeight) + scrollIndex;
                if (idx == 0) { if (currentPath.has_parent_path()) { currentPath = currentPath.parent_path(); scrollIndex = 0; refresh(); } }
                else {
                    int eIdx = idx - 1;
                    if (eIdx >= 0 && eIdx < (int)entries.size()) {
                        if (entries[eIdx].is_directory()) { currentPath = entries[eIdx].path(); scrollIndex = 0; refresh(); }
                        else selectedFile = entries[eIdx].path().string();
                    }
                }
            }
        }
    } else {
        Vector2 m = GetMousePosition();
        Rectangle btnRect = {contentBounds.x + 10, contentBounds.y + 10, 120, 30};
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(m, btnRect)) openFolderDialog();
    }
}

void FileManager::render(Rectangle bounds, Font font) {
    // --- FIX: DRAW HEADER INSIDE BOUNDS ---
    float headerH = 25.0f;
    
    // 1. Draw Header Background
    DrawRectangle(bounds.x, bounds.y, bounds.width, headerH, theme.border);
    DrawTextEx(font, "EXPLORER", {bounds.x + 5, bounds.y + 2}, Config::FONT_SIZE_UI, 1, theme.menuText);
    
    // 2. Draw Content Background
    Rectangle contentRect = {bounds.x, bounds.y + headerH, bounds.width, bounds.height - headerH};
    DrawRectangleRec(contentRect, theme.panelBg);
    DrawRectangleLinesEx(bounds, 1, theme.border);

    BeginScissorMode((int)contentRect.x, (int)contentRect.y, (int)contentRect.width, (int)contentRect.height);
        if (!isLoaded) {
            DrawTextEx(font, "No Folder.", {contentRect.x + 10, contentRect.y + 10}, Config::FONT_SIZE_UI, 1, GRAY);
            Rectangle btnRect = {contentRect.x + 10, contentRect.y + 40, 120, 30};
            bool hover = CheckCollisionPointRec(GetMousePosition(), btnRect);
            DrawRectangleRec(btnRect, hover ? theme.btnNormal : theme.border);
            DrawTextEx(font, "Open Folder", {btnRect.x + 10, btnRect.y + 5}, 18, 1, WHITE);
        } else {
            float y = contentRect.y; float x = contentRect.x + 5; Vector2 mouse = GetMousePosition();
            
            Rectangle upRect = {contentRect.x, y, contentRect.width, itemHeight};
            if (CheckCollisionPointRec(mouse, upRect)) DrawRectangleRec(upRect, theme.fileHover);
            
            if (folderIcon.id > 0) DrawTexture(folderIcon, (int)x, (int)y + 2, WHITE);
            else DrawTextEx(font, "^", {x, y}, Config::FONT_SIZE_UI, 1, theme.keyword);
            
            DrawTextEx(font, "..", {x + 25, y}, Config::FONT_SIZE_UI, 1, theme.keyword); 
            
            for (int i = 0; i < entries.size(); i++) {
                float dy = y + (i + 1 - scrollIndex) * itemHeight;
                if (dy < contentRect.y - itemHeight) continue; if (dy > contentRect.y + contentRect.height) break;
                
                Rectangle itemRect = {contentRect.x, dy, contentRect.width, itemHeight};
                if (CheckCollisionPointRec(mouse, itemRect)) DrawRectangleRec(itemRect, theme.fileHover);
                
                std::string n = entries[i].path().filename().string();
                bool isDir = entries[i].is_directory();
                Color c = isDir ? theme.folder : theme.text;
                float textX = x;
                
                if (isDir && folderIcon.id > 0) { DrawTexture(folderIcon, (int)x, (int)dy + 2, WHITE); textX += 25; } 
                else if (isDir) { DrawTextEx(font, "[D]", {x, dy}, Config::FONT_SIZE_UI, 1, theme.keyword); textX += 35; } 
                else { textX += 25; }

                DrawTextEx(font, n.c_str(), {textX, dy}, Config::FONT_SIZE_UI, 1, c);
            }
        }
    EndScissorMode();
}