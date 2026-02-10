#pragma once
#include "Globals.hpp"
#include <unordered_set>
#include <deque>

struct UndoState {
    std::vector<std::string> lines;
    int row, col;
};

struct Document {
    std::string path;
    std::string filename;
    std::vector<std::string> lines;
    
    int row = 0, col = 0;
    int scroll = 0;
    int selRowStart = -1, selColStart = -1;
    int selRowEnd = -1, selColEnd = -1;
    bool selecting = false;
    bool isDirty = false;
    std::deque<UndoState> undoStack;

    Document(std::string p = "");
};

class Editor {
private:
    std::vector<Document> docs;
    int activeTab = 0;
    Font font;
    
    float charWidth; 
    int lineHeight;
    
    float blink = 0;
    bool showCursor = true;
    float backspaceTimer = 0.0f;
    float backspaceDelay = 0.35f;
    float backspaceSpeed = 0.03f;

    std::unordered_set<std::string> keywords;
    std::unordered_set<std::string> types;

    enum class PreviewType { None, Image, Audio };
    PreviewType previewType = PreviewType::None;
    std::string previewPath = "";
    Texture2D previewTex = { 0 };
    Sound previewSound = { 0 };

    Document& currentDoc();
    void pushUndo();
    void performUndo();
    bool hasSelection(const Document& doc);
    void clearSelection(Document& doc);
    void deleteSelection(Document& doc);
    void normalizeSelection(int& r1, int& c1, int& r2, int& c2, const Document& doc);
    std::string getSelectedText(const Document& doc);
    
    void moveLeft(Document& doc);
    void moveRight(Document& doc);
    
    void deleteCharBackwards();
    void deleteWordBackwards();
    void drawLine(const Document& doc, int lineIdx, int x, int y);
    void clearPreview();

public:
    Editor();
    void init(Font f);
    void updateFontMetrics();
    void reloadFont(Font f);
    
    void createNewFile();
    void loadFile(const std::string& path);
    void setPreview(const std::string& path);
    void saveFile(); 
    void saveAs(); 

    std::string getCurrentPath();
    
    void selectAll();
    void copyToClipboard();
    void pasteFromClipboard(); 

    void update(Rectangle bounds, bool isFocused);
    void render(Rectangle bounds);
};
