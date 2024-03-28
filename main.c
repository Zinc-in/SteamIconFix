//
// Created by -Zinc- on 2024/3/22.
//Refactor by C
//

#include <locale.h>
#include "SteamIconFix_C.h"
// for windows only :(


int main() {
    setlocale(LC_CTYPE, "");
    wchar_t progFilesDir[MAX_PATH];
    DWORD size = MAX_PATH;
    if (!SUCCEEDED(RegGetValueW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\WOW6432Node\\Valve\\Steam", L"InstallPath", RRF_RT_ANY,
                                NULL, (PVOID) progFilesDir, &size))) {
        logerr(L"E Steam is not installed, exiting...\n");
        _wsystem(L"pause");
        exit(0);
    }
    wcscat_s(progFilesDir, MAX_PATH, L"\\steam\\games");
    size = MAX_PATH;

    wchar_t desktopDir[MAX_PATH];
    if (!SUCCEEDED(
            RegGetValueW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",
                         L"Desktop", RRF_RT_ANY,
                         NULL, (PVOID) desktopDir, &size))) {
        logerr(L"E Cannot find Desktop, exiting...\n");
        _wsystem(L"pause");
        exit(0);
    }
    size = MAX_PATH;

    wchar_t startMenuDir[MAX_PATH];
    if (!SUCCEEDED(
            RegGetValueW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",
                         L"Programs", RRF_RT_ANY,
                         NULL, (PVOID) startMenuDir, &size))) {
        logwrn(L"W Cannot find Start menu, skipped\n");
    }
    wcscat_s(startMenuDir, MAX_PATH, L"\\Steam");

    if (isFileExists(progFilesDir)) {
        wprintf_s(L"Found Steam icon dir in %ls\n", progFilesDir);
    } else {
        logwrn(L"W Cannot find steam icon dir, manual input>_");
        scanf_s("%ls", &progFilesDir);
        if (!isFileExists(progFilesDir)) {
            logerr(L"E Path not exist. Exiting...\n");
            _wsystem(L"pause");
            exit(0);
        }
        wcscat_s(progFilesDir, size + 12, L"\\steam\\games");
    }

    if (isFileExists(desktopDir)) {
        wprintf_s(L"Found Desktop in %ls\n", desktopDir);
    } else {
        logwrn(L"W Cannot find desktop dir, manual input>_");
        scanf_s("%ls", &desktopDir);
        if (!isFileExists(desktopDir)) {
            logerr(L"E Path not exist. Exiting...\n");
            _wsystem(L"pause");
            exit(0);
        }
    }
    if (isFileExists(startMenuDir)) {
        wprintf_s(L"Found Steam Start Menu Shortcut in %ls", startMenuDir);
    } else {
        logwrn(L"W Cannot find StartMenu Dir, skipping...\n");
    }

    char cdnChoose[2] = {"\0"};
    char *cdn[] = {"akamai", "cloudflare"};

    wprintf_s(L"\n");

    setclr(FOREGROUND_BLUE);
    wprintf_s(L"==Type what CDN u wanna use== \n");
    printf_s("0 -> %s(default) \n", cdn[0]);
    printf_s("1 -> %s \n", cdn[1]);

    do {
        wprintf_s(L"CDN[end with enter]");
        setclr(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        fgets(cdnChoose, 2, stdin);
        if (cdnChoose[0] == '\0' || cdnChoose[0] == ' ') {
            strcpy_s(cdnChoose, 2, "0");
            break;
        }
        setclr(FOREGROUND_BLUE);
        if ((strcmp(cdnChoose, "0") == 0) || (strcmp(cdnChoose, "1") == 0)) break;
        logwrn(L"Input invalid. \n");
    } while (true);
    wprintf_s(L"\n");
    printf_s("Current using CDN %s\n", cdn[strtol(cdnChoose, NULL, 10)]);
    setclr(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

    SteamApp gameList[1024];

    char buffer[64];
    size = 0;
    getDirUrlFiles(desktopDir, gameList, &size);
    getDirUrlFiles(startMenuDir, gameList, &size);
    char iconurl[256];
    sprintf_s(iconurl, 256, "http://cdn.%s.steamstatic.com/steamcommunity/public/images/apps/",
              cdn[strtol(cdnChoose, NULL, 10)]);
    unsigned short urlSize = strlen(iconurl);
    unsigned short progSize = wcslen(progFilesDir);
    for (int i = 0; i < size; ++i) {
        wprintf_s(L"Find Steam Shortcut %ls with appid %ls \n", gameList[i].iconName, gameList[i].gameId);
        wstring2string(gameList[i].gameId, buffer);
        strcpy_s(iconurl + urlSize, 256 - urlSize, buffer);
        strcat_s(iconurl, 256, "/");
        wstring2string(gameList[i].iconName, buffer);
        strcat_s(iconurl, 256, buffer);

        printf_s("- try to download icon from %s to %ls\\%ls\n", iconurl, progFilesDir, gameList[i].iconName);
        wcscpy_s(progFilesDir + progSize, MAX_PATH - progSize, L"\\");
        wcscat_s(progFilesDir, MAX_PATH, gameList[i].iconName);

        unsigned short res = downloadFile(iconurl, progFilesDir);
        switch (res) {
            case 0:
                logsuc(L"- Successful Fixed.\n");
                break;
            case 3:
                logwrn(L"Skipped, error 3, URL error\n");
                break;
            case 404:
                logwrn(L"Skipped, error 404, File Not Found\n");
                break;
            case 500 ...599:
                logerrWithCode(L"Error %d, Server Error,please change cdn", res);
                wprintf_s(L"\n");
                res = res ^ (0x8000);
                break;
            default:
                logerrWithCode(L"E download failed. Check your network! Error %d", res);
                wprintf_s(L"\n");
                res = res ^ (0x8000);
        }
        if (res > 0x8000) {
            logerr(L"E deleting downloaded files...\n");
            wchar_t delPath[280];
            swprintf_s(delPath, 280, L"del \"%ls\"", progFilesDir);
            _wsystem(delPath);
            _wsystem(L"pause");
            exit(0);
        }
    }
    wprintf_s(L"Finish Fixing, Flushing icon cache...\n");
    flushIcon();
    logsuc(L"Success!");
    _wsystem(L"pause");
    freeSteamApp(gameList,size);
    return 0;
}
