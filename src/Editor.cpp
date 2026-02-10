#include "../include/Editor.hpp"
#include "../include/FileManager.hpp" 
#include <fstream>
#include <cmath>
#include <algorithm>

Theme theme; 
AppSettings settings; 
std::vector<Toast> toastQueue;

static bool IsImageFile(const std::string& path) {
    auto lower = path;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return lower.size() >= 4 &&
           (lower.rfind(".png") == lower.size() - 4 ||
            lower.rfind(".jpg") == lower.size() - 4 ||
            lower.rfind(".jpeg") == lower.size() - 5 ||
            lower.rfind(".gif") == lower.size() - 4 ||
            lower.rfind(".bmp") == lower.size() - 4);
}

static bool IsAudioFile(const std::string& path) {
    auto lower = path;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return lower.size() >= 4 &&
           (lower.rfind(".wav") == lower.size() - 4 ||
            lower.rfind(".mp3") == lower.size() - 4 ||
            lower.rfind(".ogg") == lower.size() - 4);
}

static Texture2D LoadPreviewTexture(const std::string& path, int maxDim) {
    Image img = LoadImage(path.c_str());
    if (img.data == NULL) return {0};
    ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    int w = img.width;
    int h = img.height;
    int largest = (w > h) ? w : h;
    if (largest > maxDim && maxDim > 0) {
        float scale = (float)maxDim / (float)largest;
        int newW = (int)roundf(w * scale);
        int newH = (int)roundf(h * scale);
        ImageResize(&img, newW, newH);
    }
    Texture2D tex = LoadTextureFromImage(img);
    SetTextureFilter(tex, TEXTURE_FILTER_BILINEAR);
    UnloadImage(img);
    return tex;
}

// Helper: UTF-8 Conversion
std::string CodepointToUTF8(int cp) {
    std::string res;
    if (cp <= 0x7F) res += (char)cp;
    else if (cp <= 0x7FF) {
        res += (char)(0xC0 | ((cp >> 6) & 0x1F));
        res += (char)(0x80 | (cp & 0x3F));
    } else if (cp <= 0xFFFF) {
        res += (char)(0xE0 | ((cp >> 12) & 0x0F));
        res += (char)(0x80 | ((cp >> 6) & 0x3F));
        res += (char)(0x80 | (cp & 0x3F));
    } else if (cp <= 0x10FFFF) {
        res += (char)(0xF0 | ((cp >> 18) & 0x07));
        res += (char)(0x80 | ((cp >> 12) & 0x3F));
        res += (char)(0x80 | ((cp >> 6) & 0x3F));
        res += (char)(0x80 | (cp & 0x3F));
    }
    return res;
}

bool IsContinuationByte(unsigned char c) {
    return (c & 0xC0) == 0x80;
}

void ShowToast(const std::string& msg) {
    toastQueue.push_back({msg, 2.0f, 2.0f});
}

void SaveSettings() {
    if (!fs::exists("data")) fs::create_directory("data");
    std::ofstream out("data/settings.cfg");
    if (out.is_open()) {
        out << "fontPath=" << settings.fontPath << "\n";
        out << "shellPath=" << settings.shellPath << "\n";
        out << "cFlags=" << settings.cFlags << "\n";
        out << "imagePreview=" << (settings.imagePreview ? 1 : 0) << "\n";
        out << "audioPreview=" << (settings.audioPreview ? 1 : 0) << "\n";
        out << "fontSize=" << settings.fontSize << "\n";
        out << "sidebarW=" << settings.sidebarWidth << "\n";
        out << "layout=" << (int)settings.layout << "\n";
        out << "theme=" << settings.themeIndex << "\n";
        out.close();
    }
}

void LoadSettings() {
    std::ifstream in("data/settings.cfg");
    if (!in.is_open()) return;
    std::string line;
    while (std::getline(in, line)) {
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);
        if (key == "fontPath") settings.fontPath = val;
        else if (key == "shellPath") settings.shellPath = val;
        else if (key == "cFlags") settings.cFlags = val;
        else if (key == "imagePreview") settings.imagePreview = (val == "1");
        else if (key == "audioPreview") settings.audioPreview = (val == "1");
        else if (key == "fontSize") settings.fontSize = std::stoi(val);
        else if (key == "sidebarW") settings.sidebarWidth = std::stoi(val);
        else if (key == "layout") settings.layout = (LayoutMode)std::stoi(val);
        else if (key == "theme") settings.themeIndex = std::stoi(val);
    }
    if (settings.themeIndex < 0 || settings.themeIndex > 2) settings.themeIndex = 0;
}

void ApplyThemePreset(int index) {
    settings.themeIndex = index;
    switch(index) {
        case 0: theme = { {30,30,30,255}, {37,37,38,255}, {60,60,60,255}, {220,220,220,255}, {86,156,214,255}, {78,201,176,255}, {206,145,120,255}, {106,153,85,255}, {181,206,168,255}, {200,200,200,255}, {38,79,120,100}, {220,220,170,255}, {30,30,30,255}, {45,45,45,255}, {60,60,60,255}, {220,220,220,255}, {50,160,50,255}, {200,60,60,255}, {50,50,52,255}, {0,122,204,255} }; break;
        case 1: theme = { {40,43,53,255}, {30,32,40,255}, {50,50,50,255}, {224,226,228,255}, {147,199,99,255}, {103,140,177,255}, {236,118,0,255}, {125,140,147,255}, {255,205,34,255}, {224,226,228,255}, {60,70,80,100}, {224,226,228,255}, {40,43,53,255}, {35,38,45,255}, {50,55,65,255}, {224,226,228,255}, {147,199,99,255}, {236,118,0,255}, {45,48,58,255}, {103,140,177,255} }; break;
        case 2: theme = { {255,255,255,255}, {243,243,243,255}, {200,200,200,255}, {50,50,50,255}, {0,0,255,255}, {43,145,175,255}, {163,21,21,255}, {0,128,0,255}, {9,134,88,255}, {0,0,0,255}, {173,214,255,100}, {200,150,50,255}, {255,255,255,255}, {230,230,230,255}, {220,220,220,255}, {20,20,20,255}, {0,180,0,255}, {200,50,50,255}, {230,230,230,255}, {0,120,215,255} }; break;
        default: ApplyThemePreset(0); break;
    }
}

Document::Document(std::string p) : path(p) {
    if (path.empty()) filename = "Untitled";
    else {
        size_t pos = path.find_last_of("/\\");
        filename = (pos == std::string::npos) ? path : path.substr(pos + 1);
    }
    lines.push_back("");
}

Editor::Editor() { createNewFile(); }

void Editor::init(Font f) {
    font = f;
    keywords = {"if", "else", "while", "for", "return", "using", "namespace", "class", "true", "false", "new", "delete", "include", "void", "int", "float", "double", "bool", "char", "string", "vector", "auto", "template", "typename", "const", "static", "public", "private", "std"};
    types = {"Editor", "FileManager", "Terminal", "Theme", "Document", "vector", "string", "map", "uint8_t", "cout", "cin", "endl"};
    updateFontMetrics();
}

void Editor::reloadFont(Font f) { font = f; updateFontMetrics(); }

void Editor::updateFontMetrics() {
    Vector2 m = MeasureTextEx(font, "M", (float)settings.fontSize, 1.0f);
    charWidth = m.x; 
    lineHeight = (int)m.y;
}

static float GetGutterWidth(Font font, int fontSize, size_t lineCount) {
    int digits = (int)std::to_string(lineCount > 0 ? lineCount : 1).size();
    if (digits < 2) digits = 2;
    std::string sample(digits, '9');
    return MeasureTextEx(font, sample.c_str(), (float)fontSize, 1.0f).x + 16.0f;
}

Document& Editor::currentDoc() {
    if (docs.empty()) createNewFile();
    if (activeTab >= (int)docs.size()) activeTab = (int)docs.size() - 1;
    return docs[activeTab];
}

std::string Editor::getCurrentPath() { return currentDoc().path; }

void Editor::clearPreview() {
    if (previewTex.id > 0) UnloadTexture(previewTex);
    if (previewSound.frameCount > 0) UnloadSound(previewSound);
    previewTex = {0};
    previewSound = {0};
    previewPath.clear();
    previewType = PreviewType::None;
}

void Editor::pushUndo() {
    Document& doc = currentDoc();
    if (doc.undoStack.size() > 50) doc.undoStack.pop_front();
    doc.undoStack.push_back({doc.lines, doc.row, doc.col});
}

void Editor::performUndo() {
    Document& doc = currentDoc();
    if (!doc.undoStack.empty()) {
        UndoState state = doc.undoStack.back();
        doc.undoStack.pop_back();
        doc.lines = state.lines;
        doc.row = state.row; doc.col = state.col; doc.isDirty = true;
    }
}

// Selection & Clipboard
bool Editor::hasSelection(const Document& doc) { return doc.selRowStart != -1; }
void Editor::clearSelection(Document& doc) { doc.selRowStart = -1; doc.selecting = false; }
void Editor::normalizeSelection(int& r1, int& c1, int& r2, int& c2, const Document& doc) {
    r1 = doc.selRowStart; c1 = doc.selColStart; r2 = doc.selRowEnd; c2 = doc.selColEnd;
    if (r1 > r2 || (r1 == r2 && c1 > c2)) { std::swap(r1, r2); std::swap(c1, c2); }
}

std::string Editor::getSelectedText(const Document& doc) {
    if (!hasSelection(doc)) return "";
    int r1, c1, r2, c2; normalizeSelection(r1, c1, r2, c2, doc);
    std::string result;
    for (int i = r1; i <= r2; i++) {
        std::string line = doc.lines[i];
        int start = (i == r1) ? c1 : 0;
        int end = (i == r2) ? c2 : line.size();
        if (start < line.size()) result += line.substr(start, end - start);
        if (i != r2) result += "\n";
    }
    return result;
}

void Editor::deleteSelection(Document& doc) {
    if (!hasSelection(doc)) return;
    int r1, c1, r2, c2; normalizeSelection(r1, c1, r2, c2, doc);
    if (r1 == r2) doc.lines[r1].erase(c1, c2 - c1);
    else {
        std::string tail = doc.lines[r2].substr(c2);
        doc.lines[r1].erase(c1); doc.lines[r1] += tail;
        doc.lines.erase(doc.lines.begin() + r1 + 1, doc.lines.begin() + r2 + 1);
    }
    doc.row = r1; doc.col = c1; clearSelection(doc); doc.isDirty = true;
}

void Editor::selectAll() {
    Document& doc = currentDoc();
    if (doc.lines.empty()) return;
    doc.selRowStart = 0; doc.selColStart = 0;
    doc.selRowEnd = doc.lines.size() - 1;
    doc.selColEnd = doc.lines.back().size();
    doc.row = doc.selRowEnd; doc.col = doc.selColEnd;
    doc.selecting = true;
}

void Editor::copyToClipboard() {
    Document& doc = currentDoc();
    std::string text = getSelectedText(doc);
    if (!text.empty()) { SetClipboardText(text.c_str()); ShowToast("Copied"); }
}

void Editor::pasteFromClipboard() {
    const char* text = GetClipboardText();
    if (!text) return;
    std::string str(text);
    if (str.empty()) return;
    Document& doc = currentDoc();
    pushUndo();
    if (hasSelection(doc)) deleteSelection(doc);
    size_t pos = 0;
    while (pos < str.length()) {
        size_t next = str.find('\n', pos);
        std::string segment;
        if (next == std::string::npos) { segment = str.substr(pos); pos = str.length(); } 
        else { segment = str.substr(pos, next - pos); if (!segment.empty() && segment.back() == '\r') segment.pop_back(); pos = next + 1; }
        doc.lines[doc.row].insert(doc.col, segment); doc.col += segment.length();
        if (pos < str.length() || (next != std::string::npos)) {
            std::string rest = doc.lines[doc.row].substr(doc.col);
            doc.lines[doc.row] = doc.lines[doc.row].substr(0, doc.col);
            doc.lines.insert(doc.lines.begin() + doc.row + 1, rest);
            doc.row++; doc.col = 0;
        }
    }
    doc.isDirty = true;
}

// Navigation
void Editor::moveLeft(Document& doc) {
    if (doc.col > 0) {
        doc.col--;
        while (doc.col > 0 && IsContinuationByte(doc.lines[doc.row][doc.col])) doc.col--;
    } else if (doc.row > 0) {
        doc.row--; doc.col = doc.lines[doc.row].size();
    }
}

void Editor::moveRight(Document& doc) {
    if (doc.col < (int)doc.lines[doc.row].size()) {
        doc.col++;
        while (doc.col < (int)doc.lines[doc.row].size() && IsContinuationByte(doc.lines[doc.row][doc.col])) doc.col++;
    } else if (doc.row < (int)doc.lines.size() - 1) {
        doc.row++; doc.col = 0;
    }
}

void Editor::deleteCharBackwards() {
    Document& doc = currentDoc();
    if (doc.col > 0) {
        int originalCol = doc.col;
        moveLeft(doc);
        int bytesToDelete = originalCol - doc.col;
        doc.lines[doc.row].erase(doc.col, bytesToDelete);
        doc.isDirty = true;
    } else if (doc.row > 0) {
        doc.col = doc.lines[doc.row - 1].size();
        doc.lines[doc.row - 1] += doc.lines[doc.row];
        doc.lines.erase(doc.lines.begin() + doc.row); doc.row--; doc.isDirty = true;
    }
}

void Editor::deleteWordBackwards() {
    Document& doc = currentDoc();
    if (doc.col == 0) { deleteCharBackwards(); return; }
    std::string& line = doc.lines[doc.row];
    int start = doc.col;
    while (start > 0 && (line[start-1] == ' ' || line[start-1] == '\t')) start--;
    if (start > 0) {
        bool isAlpha = isalnum(line[start-1]) || line[start-1] == '_';
        while (start > 0) {
            bool prev = isalnum(line[start-1]) || line[start-1] == '_';
            if (prev != isAlpha) break;
            start--;
        }
    }
    line.erase(start, doc.col - start); doc.col = start; doc.isDirty = true;
}

// File IO
void Editor::createNewFile() { clearPreview(); docs.push_back(Document()); activeTab = (int)docs.size() - 1; }

void Editor::loadFile(const std::string& path) {
    clearPreview();
    for (size_t i = 0; i < docs.size(); i++) { if (docs[i].path == path) { activeTab = i; return; } }
    std::ifstream in(path);
    if (in.is_open()) {
        Document newDoc(path); newDoc.lines.clear(); std::string line;
        while (std::getline(in, line)) { if (!line.empty() && line.back() == '\r') line.pop_back(); newDoc.lines.push_back(line); }
        if (newDoc.lines.empty()) newDoc.lines.push_back("");
        Document& curr = currentDoc();
        if (curr.path.empty() && curr.lines.size()==1 && curr.lines[0].empty() && !curr.isDirty) docs[activeTab] = newDoc;
        else { docs.push_back(newDoc); activeTab = (int)docs.size()-1; }
    }
}

void Editor::setPreview(const std::string& path) {
    if (path.empty()) { clearPreview(); return; }
    if (IsImageFile(path)) {
        if (!settings.imagePreview) return;
        if (previewType == PreviewType::Image && previewPath == path) return;
        clearPreview();
        previewTex = LoadPreviewTexture(path, 1024);
        if (previewTex.id > 0) { previewType = PreviewType::Image; previewPath = path; }
    } else if (IsAudioFile(path)) {
        if (!settings.audioPreview) return;
        if (previewType == PreviewType::Audio && previewPath == path) return;
        clearPreview();
        previewSound = LoadSound(path.c_str());
        if (previewSound.frameCount > 0) { previewType = PreviewType::Audio; previewPath = path; }
    } else {
        clearPreview();
    }
}

void Editor::saveAs() {
    Document& doc = currentDoc();
    std::string newPath = SaveWindowsFileDialog(doc.filename.c_str());
    if (!newPath.empty()) {
        doc.path = newPath;
        size_t pos = doc.path.find_last_of("/\\");
        doc.filename = (pos == std::string::npos) ? doc.path : doc.path.substr(pos + 1);
        saveFile();
    }
}

void Editor::saveFile() {
    Document& doc = currentDoc();
    if (doc.path.empty()) { saveAs(); return; }
    std::ofstream out(doc.path);
    if (out.is_open()) {
        for (size_t i = 0; i < doc.lines.size(); i++) { out << doc.lines[i]; if (i < doc.lines.size() - 1) out << "\n"; }
        doc.isDirty = false; ShowToast("Saved: " + doc.filename);
    } else { ShowToast("Save Failed!"); }
}

// Update Loop
void Editor::update(Rectangle bounds, bool isFocused) {
    if (!isFocused) return;
    Document& doc = currentDoc();
    bool previewActive = (previewType != PreviewType::None);

    bool ctrl = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
    bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

    // Shortcuts
    if (ctrl) {
        if (IsKeyPressed(KEY_S)) saveFile();
        if (IsKeyPressed(KEY_Z)) { performUndo(); return; }
        if (IsKeyPressed(KEY_N)) createNewFile();
        if (IsKeyPressed(KEY_W)) { if (!docs.empty()) { docs.erase(docs.begin() + activeTab); if (activeTab >= (int)docs.size()) activeTab = (int)docs.size() - 1; if (docs.empty()) createNewFile(); } }
        if (IsKeyPressed(KEY_A)) selectAll();
        if (IsKeyPressed(KEY_C)) copyToClipboard();
        if (IsKeyPressed(KEY_V)) pasteFromClipboard();
        float wheel = GetMouseWheelMove();
        if (wheel != 0) { settings.fontSize += (int)wheel * 2; if (settings.fontSize < 10) settings.fontSize = 10; updateFontMetrics(); return; }
    } else {
        float wheel = GetMouseWheelMove();
        if (!previewActive) { doc.scroll -= (int)wheel * 3; if (doc.scroll < 0) doc.scroll = 0; }
    }

    if (!previewActive) {
    // Input text
    int c = GetCharPressed();
    if (c > 0) { pushUndo(); deleteSelection(doc); } 
    while (c > 0) {
        std::string utf8Str = CodepointToUTF8(c);
        doc.lines[doc.row].insert(doc.col, utf8Str);
        doc.col += utf8Str.length();
        if (c=='{') doc.lines[doc.row].insert(doc.col, "}");
        if (c=='(') doc.lines[doc.row].insert(doc.col, ")");
        if (c=='[') doc.lines[doc.row].insert(doc.col, "]");
        if (c=='"') doc.lines[doc.row].insert(doc.col, "\"");
        doc.isDirty = true;
        c = GetCharPressed();
    }

    // Backspace
    if ((ctrl && IsKeyPressed(KEY_BACKSPACE)) || (ctrl && IsKeyPressed(KEY_SPACE))) { pushUndo(); deleteSelection(doc); deleteWordBackwards(); } 
    else if (IsKeyDown(KEY_BACKSPACE) && !ctrl) {
        if (IsKeyPressed(KEY_BACKSPACE)) { pushUndo(); if (hasSelection(doc)) deleteSelection(doc); else deleteCharBackwards(); backspaceTimer = 0.0f; }
        else {
            backspaceTimer += GetFrameTime();
            if (backspaceTimer > backspaceDelay) {
                if (((int)((backspaceTimer - backspaceDelay)/backspaceSpeed)) > ((int)((backspaceTimer - backspaceDelay - GetFrameTime())/backspaceSpeed))) {
                    if (hasSelection(doc)) deleteSelection(doc); else deleteCharBackwards();
                }
            }
        }
    } else backspaceTimer = 0.0f;

    // Enter
    if (IsKeyPressed(KEY_ENTER)) {
        pushUndo(); deleteSelection(doc);
        std::string cur = doc.lines[doc.row]; std::string rest = cur.substr(doc.col); doc.lines[doc.row] = cur.substr(0, doc.col);
        int indent = 0; while(indent < (int)doc.lines[doc.row].size() && doc.lines[doc.row][indent] == ' ') indent++;
        if (doc.col > 0 && doc.lines[doc.row][doc.col-1] == '{') {
             if (!rest.empty() && rest[0] == '}') { doc.lines.insert(doc.lines.begin() + doc.row + 1, std::string(indent, ' ') + rest); doc.lines.insert(doc.lines.begin() + doc.row + 1, std::string(indent + 4, ' ')); doc.row++; doc.col = indent + 4; }
             else { indent += 4; doc.lines.insert(doc.lines.begin() + doc.row + 1, std::string(indent, ' ') + rest); doc.row++; doc.col = indent; }
        } else { doc.lines.insert(doc.lines.begin() + doc.row + 1, std::string(indent, ' ') + rest); doc.row++; doc.col = indent; }
        doc.isDirty = true;
    }
    
    if (IsKeyPressed(KEY_TAB) && !ctrl) { pushUndo(); deleteSelection(doc); doc.lines[doc.row].insert(doc.col, "    "); doc.col += 4; doc.isDirty = true; }

    // Navigation
    bool moved = false;
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN)) moved = true;
    if (moved) { if (shift && !doc.selecting) { doc.selecting = true; doc.selRowStart = doc.row; doc.selColStart = doc.col; } if (!shift && !doc.selecting) clearSelection(doc); }
    
    if (IsKeyPressed(KEY_LEFT)) moveLeft(doc);
    if (IsKeyPressed(KEY_RIGHT)) moveRight(doc);
    
    // Safety check for out of bounds
    if (IsKeyPressed(KEY_UP)) { 
        if (doc.row > 0) doc.row--; 
        if (doc.col > (int)doc.lines[doc.row].size()) doc.col = doc.lines[doc.row].size();
    }
    if (IsKeyPressed(KEY_DOWN)) { 
        if (doc.row < (int)doc.lines.size() - 1) doc.row++; 
        if (doc.col > (int)doc.lines[doc.row].size()) doc.col = doc.lines[doc.row].size();
    }
    
    if (shift && doc.selecting) { doc.selRowEnd = doc.row; doc.selColEnd = doc.col; }
    if (!shift && doc.selecting && moved) clearSelection(doc);
    }

    // Mouse click
    Vector2 m = GetMousePosition();
    float tabX = bounds.x; 
    float tabH = Config::TAB_HEIGHT;
    for (int i=0; i<docs.size(); i++) {
        std::string t = docs[i].filename + (docs[i].isDirty?"*":""); 
        float tW = MeasureTextEx(font, t.c_str(), Config::FONT_SIZE_UI, 1).x + 40;
        Rectangle tabR = {tabX, bounds.y, tW, tabH};
        if (CheckCollisionPointRec(m, tabR)) {
            Rectangle closeR = {tabX + tW - 25, bounds.y + 5, 20, 20};
            if (CheckCollisionPointRec(m, closeR)) {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) { docs.erase(docs.begin() + i); if (activeTab >= (int)docs.size()) activeTab = (int)docs.size()-1; if (docs.empty()) createNewFile(); return; }
            } else if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) activeTab = i;
        }
        tabX += tW + 2;
    }
    
    Rectangle contentR = {bounds.x, bounds.y + tabH, bounds.width, bounds.height - tabH};
    if (!previewActive && CheckCollisionPointRec(m, contentR)) {
        float gutterW = GetGutterWidth(font, settings.fontSize, doc.lines.size());
        float relY = m.y - contentR.y;
        float relX = m.x - contentR.x - gutterW - 4.0f;
        if (relX < 0.0f) relX = 0.0f;
        int r = (int)(relY / lineHeight) + doc.scroll; r = Clamp(r, 0, (int)doc.lines.size() - 1);
        int c = (int)round(relX / charWidth); 
        c = Clamp(c, 0, (int)doc.lines[r].size());
        
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) { doc.row = r; doc.col = c; doc.selecting = true; doc.selRowStart = r; doc.selColStart = c; doc.selRowEnd = r; doc.selColEnd = c; }
        else if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && doc.selecting) { doc.selRowEnd = r; doc.selColEnd = c; doc.row = r; doc.col = c; }
        else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) { if (doc.selRowStart == doc.selRowEnd && doc.selColStart == doc.selColEnd) clearSelection(doc); }
    }
    blink += GetFrameTime(); if (blink > 0.5f) { blink = 0; showCursor = !showCursor; }
}

void Editor::render(Rectangle bounds) {
    float tabH = Config::TAB_HEIGHT; 
    Vector2 mouse = GetMousePosition();
    float tabX = bounds.x;
    
    // Render Tabs
    for (int i=0; i<docs.size(); i++) {
        std::string title = docs[i].filename + (docs[i].isDirty ? "*" : "");
        float textW = MeasureTextEx(font, title.c_str(), Config::FONT_SIZE_UI, 1).x; 
        float tabW = textW + 40;
        Rectangle tabRect = {tabX, bounds.y, tabW, tabH}; 
        bool isHover = CheckCollisionPointRec(mouse, tabRect);
        
        DrawRectangleRec(tabRect, (i==activeTab) ? theme.tabActive : theme.tabInactive);
        if (i==activeTab) DrawRectangle((int)tabX, (int)bounds.y, (int)tabW, 2, theme.keyword);
        
        DrawTextEx(font, title.c_str(), {tabX+10, bounds.y+5}, Config::FONT_SIZE_UI, 1, (i==activeTab)?WHITE:GRAY);
        if (isHover) DrawTextEx(font, "x", {tabX + tabW - 20, bounds.y + 5}, 18, 1, theme.closeBtn);
        DrawLine((int)(tabX+tabW), (int)bounds.y, (int)(tabX+tabW), (int)(bounds.y+tabH), theme.border);
        tabX += tabW + 2;
    }
    DrawRectangle((int)tabX, (int)bounds.y, (int)(bounds.width-(tabX-bounds.x)), (int)tabH, theme.panelBg);

    // Render Content
    Rectangle content = {bounds.x, bounds.y+tabH, bounds.width, bounds.height-tabH};
    Document& doc = currentDoc(); DrawRectangleRec(content, theme.bg);
    if (previewType == PreviewType::Image && !settings.imagePreview) clearPreview();
    if (previewType == PreviewType::Audio && !settings.audioPreview) clearPreview();
    if (previewType != PreviewType::None) {
        float pad = 20.0f;
        Rectangle area = {content.x + pad, content.y + pad, content.width - pad*2, content.height - pad*2};
        DrawRectangleLinesEx(area, 1, theme.border);
        DrawTextEx(font, previewPath.c_str(), {area.x + 8, area.y + 6}, Config::FONT_SIZE_UI, 1, theme.menuText);
        if (previewType == PreviewType::Image && previewTex.id > 0) {
            float maxW = area.width - 16;
            float maxH = area.height - 48;
            float sx = maxW / (float)previewTex.width;
            float sy = maxH / (float)previewTex.height;
            float s = (sx < sy) ? sx : sy;
            if (s > 1.0f) s = 1.0f;
            float dw = previewTex.width * s;
            float dh = previewTex.height * s;
            float dx = area.x + (area.width - dw) * 0.5f;
            float dy = area.y + 36 + (maxH - dh) * 0.5f;
            DrawTexturePro(previewTex,
                           (Rectangle){0, 0, (float)previewTex.width, (float)previewTex.height},
                           (Rectangle){dx, dy, dw, dh},
                           (Vector2){0, 0}, 0.0f, WHITE);
        } else if (previewType == PreviewType::Audio) {
            Rectangle btnPlay = {area.x + 16, area.y + 48, 90, 32};
            Rectangle btnStop = {area.x + 116, area.y + 48, 90, 32};
            bool hPlay = CheckCollisionPointRec(mouse, btnPlay);
            bool hStop = CheckCollisionPointRec(mouse, btnStop);
            DrawRectangleRec(btnPlay, hPlay ? theme.btnNormal : theme.border);
            DrawRectangleRec(btnStop, hStop ? theme.btnNormal : theme.border);
            DrawTextEx(font, "Play", {btnPlay.x + 20, btnPlay.y + 6}, 18, 1, WHITE);
            DrawTextEx(font, "Stop", {btnStop.x + 20, btnStop.y + 6}, 18, 1, WHITE);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && hPlay && previewSound.frameCount > 0) PlaySound(previewSound);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && hStop && previewSound.frameCount > 0) StopSound(previewSound);
        }
        return;
    }
    float gutterW = GetGutterWidth(font, settings.fontSize, doc.lines.size());
    DrawRectangleRec({content.x, content.y, gutterW, content.height}, theme.panelBg);
    DrawLine((int)(content.x + gutterW), (int)content.y, (int)(content.x + gutterW), (int)(content.y + content.height), theme.border);
    BeginScissorMode((int)content.x, (int)content.y, (int)content.width, (int)content.height);
        int vis = (int)(content.height / lineHeight) + 1;
        for (int i=0; i<vis; i++) {
            int idx = i + doc.scroll; if (idx >= doc.lines.size()) break;
            int lineY = (int)(content.y + i*lineHeight);
            std::string num = std::to_string(idx + 1);
            float numW = MeasureTextEx(font, num.c_str(), settings.fontSize, 1.0f).x;
            DrawTextEx(font, num.c_str(), {content.x + gutterW - 8.0f - numW, (float)lineY}, (float)settings.fontSize, 1.0f, theme.comment);
            drawLine(doc, idx, (int)(content.x + gutterW + 4), lineY);
        }
        if (showCursor) {
            std::string sub = doc.lines[doc.row].substr(0, doc.col);
            float cursorX = MeasureTextEx(font, sub.c_str(), settings.fontSize, 1.0f).x;
            int cx = (int)(content.x + gutterW + 4 + cursorX);
            int cy = (int)(content.y + (doc.row - doc.scroll) * lineHeight);
            if (cy >= content.y && cy < content.y + content.height) DrawRectangle(cx, cy, 2, lineHeight, theme.cursor);
        }
    EndScissorMode();
}

void Editor::drawLine(const Document& doc, int lineIdx, int x, int y) {
    std::string text = doc.lines[lineIdx];
    float cx = (float)x;
    
    // Highlight selection
    if (hasSelection(doc)) {
        int r1, c1, r2, c2; normalizeSelection(r1, c1, r2, c2, doc);
        if (lineIdx >= r1 && lineIdx <= r2) {
            float startX = 0, width = 0;
            if (lineIdx == r1) startX = MeasureTextEx(font, text.substr(0, c1).c_str(), settings.fontSize, 1.0f).x;
            if (lineIdx == r2) width = MeasureTextEx(font, text.substr(0, c2).c_str(), settings.fontSize, 1.0f).x - startX;
            else width = MeasureTextEx(font, text.c_str(), settings.fontSize, 1.0f).x - startX + 10; 
            
            if (lineIdx > r1 && lineIdx < r2) { startX = 0; width = MeasureTextEx(font, text.c_str(), settings.fontSize, 1.0f).x + 10; }
            DrawRectangle((int)(cx + startX), y, (int)width, lineHeight, theme.selection);
        }
    }
    
    // Draw text with highlighting
    size_t pos = 0;
    while (pos < text.length()) {
        size_t nextSpace = text.find_first_of(" \t", pos);
        if (nextSpace == std::string::npos) nextSpace = text.length();
        
        std::string word = text.substr(pos, nextSpace - pos);
        Color c = theme.text;
        if (keywords.count(word)) c = theme.keyword;
        else if (types.count(word)) c = theme.type;
        else if (isdigit(word[0])) c = theme.number;
        else if (word.find("//") == 0) c = theme.comment;
        else if (word.find("\"") != std::string::npos) c = theme.string;

        DrawTextEx(font, word.c_str(), {cx, (float)y}, (float)settings.fontSize, 1.0f, c);
        cx += MeasureTextEx(font, word.c_str(), settings.fontSize, 1.0f).x;
        
        // Add spacing manual adjust
        if (word.length() > 0 && nextSpace < text.length()) cx += 1.0f; 

        if (nextSpace < text.length()) {
            char delim = text[nextSpace];
            std::string dStr(1, delim);
            DrawTextEx(font, dStr.c_str(), {cx, (float)y}, (float)settings.fontSize, 1.0f, theme.text);
            cx += MeasureTextEx(font, dStr.c_str(), settings.fontSize, 1.0f).x;
            if (nextSpace + 1 < text.length()) cx += 1.0f;
            pos = nextSpace + 1;
        } else pos = nextSpace;
    }
}
