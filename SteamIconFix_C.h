//
// Created by -Zinc- on 2024/3/22.
//

#ifndef STEAMICONFIX_STEAMICONFIX_C_H
#define STEAMICONFIX_STEAMICONFIX_C_H

#include <stdio.h>
#include <stdbool.h>
#include <sec_api/wchar_s.h>
#include <winsock2.h>
#include <windows.h>

typedef struct SteamApp {
    wchar_t *gameId;
    wchar_t *iconName;
} SteamApp;
///////////////////////////////////////////////////////////////////
bool isFileExists(const wchar_t *path);

size_t curlWriteFunc(void *ptr, size_t size, size_t nmemb, FILE *stream);

unsigned short downloadFile(const char *url, const wchar_t *path);

void flushIcon();

void setclr(unsigned short clr);

void logsuc(const wchar_t *str);

void logwrn(const wchar_t *str);

void logerr(const wchar_t *str);

void logerrWithCode(const wchar_t *format,int errorCode);

bool getDirUrlFiles(wchar_t *path, SteamApp *files,unsigned long*size);

int wstring2string(const wchar_t *wstr, char *str);

void freeSteamApp(SteamApp *steamApp,unsigned long size);

#endif //STEAMICONFIX_STEAMICONFIX_C_H
