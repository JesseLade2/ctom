#include "../include/FileManager.hpp"

void FileManager::init() { 
    isLoaded = false; 
    entries.clear();
    
    // Load folder icon with alpha support
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
    if (!path.empty()) {
        selectedFile = path;
    }
}

void FileManager::refresh() {
    if (!isLoaded) return;
    entries.clear();
    try {
        for (const auto& entry : fs::directory_iterator(currentPath)) entries.push_back(entry);
        // Sort folders first
        std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
            if (a.is_directory() != b.is_directory()) return a.is_directory();
            return a.path().filename() < b.path().filename();
        });
    } catch (...) { isLoaded = false; }
}

std::string FileManager::popSelectedFile() {
    std::string s = selectedFile; 
    selectedFile = ""; 
    return s;
}

void FileManager::update(Rectangle bounds, bool isFocused) {
    if (isLoaded) {
        if (isFocused) {
            float wheel = GetMouseWheelMove();
            scrollIndex -= (int)wheel; 
            if (scrollIndex < 0) scrollIndex = 0;
        }
        // Handle clicks
        if (isFocused && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 m = GetMousePosition();
            if (CheckCollisionPointRec(m, bounds)) {
                int idx = (int)((m.y - bounds.y) / itemHeight) + scrollIndex;
                
                // Click on ".."
                if (idx == 0) { 
                    if (currentPath.has_parent_path()) { 
                        currentPath = currentPath.parent_path(); 
                        scrollIndex = 0; 
                        refresh(); 
                    } 
                }
                else {
                    int eIdx = idx - 1;
                    if (eIdx >= 0 && eIdx < (int)entries.size()) {
                        if (entries[eIdx].is_directory()) { 
                            currentPath = entries[eIdx].path(); 
                            scrollIndex = 0; 
                            refresh(); 
                        }
                        else {
                            selectedFile = entries[eIdx].path().string();
                        }
                    }
                }
            }
        }
    } else {
        // Show open button if not loaded
        Vector2 m = GetMousePosition();
        Rectangle btnRect = {bounds.x + 10, bounds.y + 40, 120, 30};
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(m, btnRect)) openFolderDialog();
    }
}

void FileManager::render(Rectangle bounds, Font font) {
    DrawRectangleRec(bounds, theme.panelBg); 
    DrawRectangleLinesEx(bounds, 1, theme.border);
    
    // Header
    DrawRectangle(bounds.x, bounds.y - 25, bounds.width, 25, theme.border);
    DrawTextEx(font, "EXPLORER", {bounds.x + 5, bounds.y - 22}, Config::FONT_SIZE_UI, 1, theme.menuText);
    
    BeginScissorMode((int)bounds.x, (int)bounds.y, (int)bounds.width, (int)bounds.height);
        if (!isLoaded) {
            DrawTextEx(font, "No Folder.", {bounds.x + 10, bounds.y + 10}, Config::FONT_SIZE_UI, 1, GRAY);
            Rectangle btnRect = {bounds.x + 10, bounds.y + 40, 120, 30};
            bool hover = CheckCollisionPointRec(GetMousePosition(), btnRect);
            DrawRectangleRec(btnRect, hover ? theme.btnNormal : theme.border);
            DrawTextEx(font, "Open Folder", {btnRect.x + 10, btnRect.y + 5}, 18, 1, WHITE);
        } else {
            float y = bounds.y; 
            float x = bounds.x + 5; 
            Vector2 mouse = GetMousePosition();
            
            // Draw ".."
            Rectangle upRect = {bounds.x, y, bounds.width, itemHeight};
            if (CheckCollisionPointRec(mouse, upRect)) DrawRectangleRec(upRect, theme.fileHover);
            
            if (folderIcon.id > 0) DrawTexture(folderIcon, (int)x, (int)y + 2, WHITE);
            else DrawTextEx(font, "^", {x, y}, Config::FONT_SIZE_UI, 1, theme.keyword);
            
            DrawTextEx(font, "..", {x + 25, y}, Config::FONT_SIZE_UI, 1, theme.keyword); 
            
            // Draw files
            for (int i = 0; i < entries.size(); i++) {
                float dy = y + (i + 1 - scrollIndex) * itemHeight;
                if (dy < bounds.y - itemHeight) continue; 
                if (dy > bounds.y + bounds.height) break;
                
                Rectangle itemRect = {bounds.x, dy, bounds.width, itemHeight};
                if (CheckCollisionPointRec(mouse, itemRect)) DrawRectangleRec(itemRect, theme.fileHover);
                
                std::string n = entries[i].path().filename().string();
                bool isDir = entries[i].is_directory();
                Color c = isDir ? theme.folder : theme.text;
                float textX = x;
                
                if (isDir && folderIcon.id > 0) {
                    DrawTexture(folderIcon, (int)x, (int)dy + 2, WHITE); 
                    textX += 25; 
                } else if (isDir) {
                    DrawTextEx(font, "[D]", {x, dy}, Config::FONT_SIZE_UI, 1, theme.keyword);
                    textX += 35;
                } else {
                    textX += 25; 
                }

                DrawTextEx(font, n.c_str(), {textX, dy}, Config::FONT_SIZE_UI, 1, c);
            }
        }
    EndScissorMode();

}
