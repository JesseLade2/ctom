#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#include <string>
#include <iostream>
#include <direct.h> 

// Get current working directory
std::string GetCurrentDir() {
    char buff[MAX_PATH];
    _getcwd(buff, MAX_PATH);
    return std::string(buff);
}

std::string OpenWindowsFolderPicker() {
    HRESULT hr = CoInitialize(NULL);
    char path[MAX_PATH];
    BROWSEINFOA bi = { 0 };
    bi.hwndOwner = GetActiveWindow();
    bi.lpszTitle = "Select Folder";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    
    LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
    std::string result = "";
    if (pidl != 0) {
        if (SHGetPathFromIDListA(pidl, path)) result = std::string(path);
        CoTaskMemFree(pidl);
    }
    return result;
}

std::string OpenWindowsFilePicker(const char* initialDir) {
    HRESULT hr = CoInitialize(NULL);
    char filename[MAX_PATH] = {0};
    OPENFILENAMEA ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetActiveWindow();
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = "Open File";
    if (initialDir) ofn.lpstrInitialDir = initialDir;
    ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    
    if (GetOpenFileNameA(&ofn)) return std::string(filename);
    return "";
}

std::string SaveWindowsFileDialog(const char* defaultName) {
    HRESULT hr = CoInitialize(NULL);
    char filename[MAX_PATH] = {0};
    if (defaultName) snprintf(filename, sizeof(filename), "%s", defaultName);

    OPENFILENAMEA ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetActiveWindow();
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = "Save File As";
    ofn.lpstrFilter = "C++ Source (*.cpp)\0*.cpp\0Headers (*.hpp;*.h)\0*.hpp;*.h\0All Files (*.*)\0*.*\0";
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

    if (GetSaveFileNameA(&ofn)) return std::string(filename);
    return ""; 
}

#else
// Linux placeholders
#include <string>
std::string OpenWindowsFolderPicker() { return ""; }
std::string OpenWindowsFilePicker(const char* d) { return ""; }
std::string SaveWindowsFileDialog(const char* d) { return ""; }
#endif