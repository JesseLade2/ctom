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

#elif defined(__APPLE__)
#include <string>
#include <cstdio>
#include <cstdlib>

static std::string RunAppleScript(const std::string& script) {
    std::string cmd = "osascript -e '" + script + "'";
    std::string result;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "";
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        result += buffer;
    }
    pclose(pipe);
    // Trim trailing newline
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) {
        result.pop_back();
    }
    return result;
}

std::string OpenWindowsFolderPicker() {
    return RunAppleScript("POSIX path of (choose folder)");
}

std::string OpenWindowsFilePicker(const char* initialDir) {
    if (initialDir && initialDir[0] != '\0') {
        std::string script = "set f to choose file default location POSIX file \"" + std::string(initialDir) + "\"\nPOSIX path of f";
        return RunAppleScript(script);
    }
    return RunAppleScript("POSIX path of (choose file)");
}

std::string SaveWindowsFileDialog(const char* defaultName) {
    if (defaultName && defaultName[0] != '\0') {
        std::string script = "set f to choose file name with prompt \"Save File As\" default name \"" + std::string(defaultName) + "\"\nPOSIX path of f";
        return RunAppleScript(script);
    }
    return RunAppleScript("POSIX path of (choose file name with prompt \"Save File As\")");
}

#elif defined(__linux__)
// Linux (zenity) dialogs
#include <string>
#include <cstdio>
#include <cstdlib>

static std::string RunZenity(const std::string& args) {
    std::string cmd = "zenity " + args;
    std::string result;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "";
    char buffer[512];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        result += buffer;
    }
    pclose(pipe);
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) {
        result.pop_back();
    }
    return result;
}

std::string OpenWindowsFolderPicker() {
    return RunZenity("--file-selection --directory --title=\"Select Folder\"");
}

std::string OpenWindowsFilePicker(const char* initialDir) {
    if (initialDir && initialDir[0] != '\0') {
        std::string arg = std::string("--file-selection --title=\"Open File\" --filename=\"") + initialDir + "/\"";
        return RunZenity(arg);
    }
    return RunZenity("--file-selection --title=\"Open File\"");
}

std::string SaveWindowsFileDialog(const char* defaultName) {
    if (defaultName && defaultName[0] != '\0') {
        std::string arg = std::string("--file-selection --save --confirm-overwrite --title=\"Save File As\" --filename=\"") +
                          defaultName + "\"";
        return RunZenity(arg);
    }
    return RunZenity("--file-selection --save --confirm-overwrite --title=\"Save File As\"");
}
#else
// Fallback placeholders
#include <string>
std::string OpenWindowsFolderPicker() { return ""; }
std::string OpenWindowsFilePicker(const char* d) { return ""; }
std::string SaveWindowsFileDialog(const char* d) { return ""; }
#endif
