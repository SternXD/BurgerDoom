#include "Game/DoomMain.h"
#include "Base/CmdLine.h"
#include <vector>
#include <string>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#pragma comment(lib, "shell32.lib")
#include <Windows.h>
#include <shellapi.h>

// atsb: extended with cmdline args to launch -file

int WINAPI wWinMain(
    [[maybe_unused]] HINSTANCE hInstance,
    [[maybe_unused]] HINSTANCE hPrevInstance,
    [[maybe_unused]] LPWSTR lpCmdLine,
    [[maybe_unused]] int nCmdShow
) {
    int argc = 0;
    LPWSTR* wargv = CommandLineToArgvW(GetCommandLineW(), &argc);
    std::vector<std::string> argvUtf8;
    argvUtf8.reserve(argc);
    if (wargv) {
        for (int i=0;i<argc;i++) {
            int bytes = WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, nullptr, 0, nullptr, nullptr);
            std::string s(bytes?bytes-1:0,'\0');
            if (bytes>0) WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, &s[0], bytes, nullptr, nullptr);
            argvUtf8.push_back(std::move(s));
        }
        LocalFree(wargv);
    }
    std::vector<char*> argv;
    argv.reserve(argvUtf8.size());
    for (auto& s : argvUtf8) argv.push_back(&s[0]);
    CmdLine::init((int)argv.size(), argv.data());
    D_DoomMain();
    return 0;
}
