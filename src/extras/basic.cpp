#include "basic.h"

uint16_t toNum(char num) {
    if (isdigit(num)) {
        return num - '0';
    }
    return 0;
}

char clearEnd(std::string& str) {
    char ret = null;
    size_t len = str.length();
    for (unsigned char i = 1; i < 3; i++) {
        if ((len >= i) && (str[len - i] == 13 || str[len - i] == 10)) {
            ret = str[len - i];
            str.pop_back();
        }
    }
    return ret;
}
void replace(std::string& str, const std::string find, const char* replace) {
    str.replace(str.find(find), find.length(), replace);
}

const char* dateStamp() {
    time_t now = CHRONO::system_clock::to_time_t(CHRONO::system_clock::now());
//#ifdef _WIN32
//    char t[26]; 
//    ctime_s(t, sizeof(t), &now);
//#else
//    char* t = ctime(&now);
//#endif
#pragma warning(suppress : 4996)
    char* t = ctime(&now);
    t[strlen(t) - 1] = '\0';
    return t;
}

const char* dateStamp(time_t* tme) {
//#ifdef _WIN32
//    char t[26];
//    ctime_s(t, sizeof(t), tme);
//#else
//    char* t = ctime(tme);
//#endif
#pragma warning(suppress : 4996)
    char* t = ctime(tme);
    t[strlen(t) - 1] = '\0';
    return t;
}

const char* safeNull(const char* str) {
    return str ? str : "(nullptr)";
}
size_t safeLen(const char* str) {
    return str ? strlen(str) : 0;
}

std::string withTime(const char* message) {
    std::string ret(dateStamp());
    ret.reserve(strlen(message) + 4);
    ret.append(" : ");
    ret.append(message);
    return ret;
}

void exitError(const char* message) {
    perror(withTime(message).c_str());
    exit(EXIT_FAILURE);
}