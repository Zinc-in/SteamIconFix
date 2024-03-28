//
// Created by -Zinc- on 2024/3/22.
//Refactor by C
//
#define CURL_STATICLIB

#include "include/curl.h"
#include "SteamIconFix_C.h"

int wstring2string(const wchar_t *wstr, char *str) {
    int len = WideCharToMultiByte(CP_ACP, 0, wstr, (int) wcslen(wstr), NULL, 0, NULL, NULL);
    WideCharToMultiByte(CP_ACP, 0, wstr, (int) wcslen(wstr), str, len, NULL, NULL);
    str[len] = '\0';
    return len;
}

bool isArrayElementDuplicate(const SteamApp *steamApps, const unsigned int size, const wchar_t *gameId) {
    for (int i = 0; i < size; ++i) {
        if (wcscmp((steamApps + i)->gameId, gameId) == 0) return true;
    }
    return false;
}

bool isFileExists(const wchar_t *path) {
    DWORD dwAttr = GetFileAttributesW(path);
    if (dwAttr == INVALID_FILE_ATTRIBUTES) return false;
    return true;
}

size_t curlWriteFunc(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return fwrite(ptr, size, nmemb, stream);
}

unsigned short downloadFile(const char *url, const wchar_t *path) {
    CURL *curl = curl_easy_init();
    FILE *ofile;
    long responseCode;
    if (curl) {
        errno_t err = _wfopen_s(&ofile, path, L"wb");
        if (err != 0) {
            logerr(L"Can't open the file,path:");
            logerr(path);
            logerr(L"\n");
            return -2;
        }
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) ofile);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteFunc);
        CURLcode res = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_HTTP_CODE, &responseCode);

        fclose(ofile);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) return res;
        if (responseCode >= 400)
            return responseCode;
        return 0;
    }
    return -1;
}

void flushIcon() {
    system("taskkill /f /im explorer.exe");
    system("attrib -h -s -r \"%userprofile%\\AppData\\Local\\IconCache.db\"");
    system("del /f \"%userprofile%\\AppData\\Local\\IconCache.db\"");
    system("attrib /s /d -h -s -r \"%userprofile%\\AppData\\Local\\Microsoft\\Windows\\Explorer\\*\"");
    system("del /f \"%userprofile%\\AppData\\Local\\Microsoft\\Windows\\Explorer\\thumbcache_32.db\"");
    system("del /f \"%userprofile%\\AppData\\Local\\Microsoft\\Windows\\Explorer\\thumbcache_96.db\"");
    system("del /f \"%userprofile%\\AppData\\Local\\Microsoft\\Windows\\Explorer\\thumbcache_102.db\"");
    system("del /f \"%userprofile%\\AppData\\Local\\Microsoft\\Windows\\Explorer\\thumbcache_256.db\"");
    system("del /f \"%userprofile%\\AppData\\Local\\Microsoft\\Windows\\Explorer\\thumbcache_1024.db\"");
    system("del /f \"%userprofile%\\AppData\\Local\\Microsoft\\Windows\\Explorer\\thumbcache_idx.db\"");
    system("del /f \"%userprofile%\\AppData\\Local\\Microsoft\\Windows\\Explorer\\thumbcache_sr.db\"");
    system("echo y　reg delete \"HKEY_CLASSES_ROOT\\Local Settings\\Software\\Microsoft\\Windows\\CurrentVersion\\TrayNotify\" /v IconStreams");
    system("echo y　reg delete \"HKEY_CLASSES_ROOT\\Local Settings\\Software\\Microsoft\\Windows\\CurrentVersion\\TrayNotify\" /v PastIconsStream");
    system("start explorer");
}

void setclr(unsigned short clr) {
    HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hCon, clr);
}


void logsuc(const wchar_t *str) {
    setclr(FOREGROUND_GREEN);
    wprintf_s(L"%ls", str);
    setclr(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

void logwrn(const wchar_t *str) {
    setclr(FOREGROUND_RED | FOREGROUND_GREEN);
    fwprintf_s(stderr, L"%ls", str);
    setclr(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

void logerr(const wchar_t *str) {
    setclr(FOREGROUND_RED);
    fwprintf_s(stderr, L"%ls", str);
    setclr(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

void logerrWithCode(const wchar_t *format, const int errorCode) {
    setclr(FOREGROUND_RED);
    fwprintf_s(stderr, format, errorCode);
    setclr(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

bool getDirUrlFiles(wchar_t *path, SteamApp *files, unsigned long *size) {
    wchar_t buffer[MAX_PATH];
    WIN32_FIND_DATAW ffd;
    long len = (long) wcslen(path);
    wcscat_s(path, MAX_PATH * 2, L"\\*.url");
    HANDLE hFind = FindFirstFileW(path, &ffd);
    if (INVALID_HANDLE_VALUE == hFind) {
        logwrn(L"Can't open the file! \n");
        return false;
    }
    do {
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        wcscpy_s(path + len + 1, 2 * MAX_PATH - len - 1, ffd.cFileName);
        GetPrivateProfileStringW(L"InternetShortcut", L"URL", NULL, buffer, sizeof(buffer), path);
        if (wcschr(buffer, L':') == NULL || wcsrchr(buffer, L'/') == NULL) continue;
        if (wcsstr(buffer, L"steam") == NULL) continue; // not a steam shortcut
        wmemmove_s(buffer, sizeof(buffer), buffer + 18, wcslen(buffer) - 17);
        if (isArrayElementDuplicate(files, *size, buffer)) continue;
        files[*size].gameId = calloc(wcslen(buffer) + 1, sizeof(wchar_t));
        wcscpy_s(files[*size].gameId, wcslen(buffer) + 1, buffer);
        /////////////////////////////////////////////////////////////////
        GetPrivateProfileStringW(L"InternetShortcut", L"IconFile", NULL, buffer, sizeof(buffer), path);
        if (wcsrchr(buffer, L'\\') == NULL) {
            logwrn(L"E invalid shortcut");
            free(files[*size].gameId);
            continue;
        }
        wmemmove_s(buffer, sizeof(buffer), wcsrchr(buffer, L'\\') + 1, wcslen(wcsrchr(buffer, L'\\') + 1) + 1);
        files[*size].iconName = calloc(wcslen(buffer) + 1, sizeof(wchar_t));
        wcscpy_s(files[*size].iconName, wcslen(buffer) + 1, buffer);
        (*size)++;
    } while (FindNextFileW(hFind, &ffd) != 0);
    FindClose(hFind);
    return true;
}

void freeSteamApp(SteamApp *steamApp, unsigned long size) {
    for (int i = 0; i < size; ++i) {
        free(steamApp[i].gameId);
        free(steamApp[i].iconName);
    }
}
