//
// Created by -Zinc- on 2024/3/10.
//

#include "../header/steamIconFix.h"
BOOL isFileExists(const string& path) {
    DWORD dwAttr = GetFileAttributes((LPCSTR)path.c_str());
    if (dwAttr == 0xFFFFFFFF) return false;
    return true;
}

bool getDirFiles(const string& path, vector<string> &files) {
    DIR *dir;
    dirent *ptr;
    struct stat s{};

    if((dir = opendir(path.c_str())) == nullptr) {
        cerr << "E[GetDirFiles] Failed open dir." << endl;
        return false;
    }

    while((ptr = readdir(dir)) != nullptr) {
        if(strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) continue; // cur or pat dir
        stat(ptr->d_name, &s);
        if(s.st_mode & S_IFDIR) continue;

        files.emplace_back(ptr->d_name);
    }

    closedir(dir);
    return true;
}

/*
BOOL downloadFile(const string& url, const string& path) {
    if(!isFileExists(path)) return false;
    if(URLDownloadToFile(nullptr, (LPCSTR)url.c_str(), (LPCSTR)path.c_str(), 0, nullptr) == S_OK) return true;
    return false;
}
*/

size_t curlWriteFunc(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return fwrite(ptr, size, nmemb, stream);
}

int downloadFile(const string& url, const string& path) {
    CURL *curl = curl_easy_init();
    if(curl) {
        FILE *ofile = fopen(path.c_str(), "wb");
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, ofile);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteFunc);
        CURLcode res = curl_easy_perform(curl);

        fclose(ofile);
        curl_easy_cleanup(curl);

        if(res != CURLE_OK) return res;
        return 0;
    }
    return -1;
}

void flushIcon() {
    system("taskkill /f /im explorer.exe");
    system(R"(attrib -h -s -r "%userprofile%\AppData\Local\IconCache.db")");
    system(R"(del /f "%userprofile%\AppData\Local\IconCache.db")");
    system(R"(attrib /s /d -h -s -r "%userprofile%\AppData\Local\Microsoft\Windows\Explorer\*")");
    system(R"(del /f "%userprofile%\AppData\Local\Microsoft\Windows\Explorer\thumbcache_32.db")");
    system(R"(del /f "%userprofile%\AppData\Local\Microsoft\Windows\Explorer\thumbcache_96.db")");
    system(R"(del /f "%userprofile%\AppData\Local\Microsoft\Windows\Explorer\thumbcache_102.db")");
    system(R"(del /f "%userprofile%\AppData\Local\Microsoft\Windows\Explorer\thumbcache_256.db")");
    system(R"(del /f "%userprofile%\AppData\Local\Microsoft\Windows\Explorer\thumbcache_1024.db")");
    system(R"(del /f "%userprofile%\AppData\Local\Microsoft\Windows\Explorer\thumbcache_idx.db")");
    system(R"(del /f "%userprofile%\AppData\Local\Microsoft\Windows\Explorer\thumbcache_sr.db")");
    system("echo y��reg delete \"HKEY_CLASSES_ROOT\\Local Settings\\Software\\Microsoft\\Windows\\CurrentVersion\\TrayNotify\" /v IconStreams");
    system("echo y��reg delete \"HKEY_CLASSES_ROOT\\Local Settings\\Software\\Microsoft\\Windows\\CurrentVersion\\TrayNotify\" /v PastIconsStream");
    system("start explorer");
}

void setclr(unsigned short clr) {
    HANDLE hCon =GetStdHandle(STD_OUTPUT_HANDLE); //��ȡ���������
    SetConsoleTextAttribute(hCon,clr); //�����ı�������ɫ
}

void logsuc(const char* str) {
    setclr(10);
    cout << str << endl;
    setclr(15);
}

void logwrn(const char* str) {
    setclr(14);
    cerr << str << endl;
    setclr(15);
}

void logerr(const char* str) {
    setclr(4);
    cerr << str << endl;
    setclr(15);
}
