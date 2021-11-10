#include "program.h"

Pdir& progdir = Pdir::get();

Pdir& Pdir::get() {
    static Pdir global;
    return global;
}

const char* Pdir::setDir(const std::string& pname) {
    this->dir = pname.substr(0, pname.find_last_of(s_dir));
    return this->dir.c_str();
}
const char* Pdir::getDir() {
    return this->dir.c_str();
}
std::string Pdir::getDirSlash() {
    return (this->dir + s_dir);
}
std::string Pdir::getItem(const std::string& itempath) {
    return this->dir + itempath;
}
std::string Pdir::getRawItem(const std::string& item) {
    return (this->dir + s_dir + item);
}
std::string* Pdir::intDir() {
    return &(this->dir);
}

std::string Pdir::directory(const std::string& fullpath) {
    return fullpath.substr(0, fullpath.find_last_of(s_dir));
}
std::string Pdir::s_directory(const std::string& fullpath) {
    return fullpath.substr(0, fullpath.find_last_of(s_dir) + 1);
}



void ArgsHandler::Variable::extractBool(const char* str) {
    std::istringstream text(str);
    text >> *(static_cast<bool*>(this->data));
}
void ArgsHandler::Variable::extractChar(const char* str) {
    std::istringstream text(str);
    text >> *(static_cast<char*>(this->data));
}
void ArgsHandler::Variable::extractInt(const char* str) {
    std::istringstream text(str);
    text >> *(static_cast<int*>(this->data));
}
void ArgsHandler::Variable::extractUint(const char* str) {
    std::istringstream text(str);
    text >> *(static_cast<uint*>(this->data));
}
void ArgsHandler::Variable::extractFloat(const char* str) {
    std::istringstream text(str);
    text >> *(static_cast<float*>(this->data));
}
void ArgsHandler::Variable::extractLong(const char* str) {
    std::istringstream text(str);
    text >> *(static_cast<long*>(this->data));
}
void ArgsHandler::Variable::extractStr(const char* str) {
    *(static_cast<std::string*>(this->data)) = str;
}

void ArgsHandler::Variable::extract(const char* str) {
    (this->*ext)(str);
}

ArgsHandler& ArgsHandler::get() {
    static ArgsHandler global;
    return global;
}

std::unordered_map<std::string, ArgsHandler::Variable>* ArgsHandler::getVars() {
    return &(this->vars);
}
void ArgsHandler::insertVars(std::initializer_list< std::pair<const std::string, Variable> > list) {
    this->vars.insert(list);
}
uint ArgsHandler::parse(int argc, char* argv[]) {
    uint noid = 0;
    std::string buffer;
    for (int i = 1; i < argc; i++) {
        std::istringstream arg(argv[i]);
        std::getline(arg, buffer, '=');
        auto result = this->vars.find(buffer);
        arg >> buffer;
        if (result != this->vars.end()) {
            result->second.extract(buffer.c_str());
        }
        else {
            noid += 1;
        }
    }
    return noid;
}