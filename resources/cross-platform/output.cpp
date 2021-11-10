#include "output.h"

#define OMODE std::ios::openmode
olstream::olstream() : 
    fmode(0), stream(&std::cout), fname(nullptr), omodes(std::ios::out) {}
olstream::olstream(const char* file, const OMODE modes) :
    fmode(1), stream(&std::cout), omodes(modes) 
{
    this->fname = new char[strlen(file)+1];
    strcpy(this->fname, file); 
}
olstream::olstream(const OMODE modes) : 
    fmode(0), stream(&std::cout), fname(nullptr), omodes(modes) {}
olstream::olstream(std::ostream* stream) : 
    fmode(0), stream(stream), fname(nullptr), omodes(std::ios::out) {}
olstream::olstream(std::ostream* stream, const char* file, const OMODE modes) : 
    fmode(0), stream(stream), omodes(modes) 
{
    this->fname = new char[strlen(file)+1];
    strcpy(this->fname, file); 
}
olstream::olstream(const olstream& other) : 
    fmode(other.fmode), stream(other.stream), omodes(other.omodes) 
{ 
    if (other.fname) {
        this->fname = new char[strlen(other.fname) + 1];
        strcpy(this->fname, other.fname);
    }
    else {
        this->fname = nullptr;
    }
}
olstream::olstream(olstream&& other) : 
    fmode(other.fmode), file(std::move(other.file)), stream(other.stream), fname(other.fname), omodes(other.omodes) 
{ 
    if(this->checkStream()) { 
        other.stream = nullptr; 
    } 
    other.fname = nullptr;
}
olstream::~olstream() { 
    delete[] this->fname; 
}

void olstream::operator=(const olstream& other) {
    this->fmode = other.fmode;
    this->stream = other.stream;
    this->omodes = other.omodes;

    if (other.fname) {
        delete[] this->fname;
        this->fname = new char[strlen(other.fname)+1];
        strcpy(this->fname, other.fname);
    }
    else {
        this->fname = nullptr;
    }

    this->checkStream();
}
void olstream::operator=(olstream&& other) {
    if (this != &other) {
        this->fmode = other.fmode;
        this->file = std::move(other.file);
        this->stream = other.stream;
        delete[] this->fname;
        this->fname = other.fname;
        other.fname = nullptr;
        this->omodes = other.omodes;

        if(this->checkStream()) {
            other.stream = nullptr;
        }
    }
}

bool olstream::checkStream() {
    if (this->stream == nullptr) {
        this->stream = &std::cout;
        return 0;
    }
    return 1;
}
std::ostream& olstream::streamGuard() {
    if (this->stream != nullptr) {
        return *(this->stream);
    }
    return std::cout;
}

void olstream::setModes(const OMODE modes) {
    this->omodes = modes;
}
void olstream::addModes(const OMODE modes) {
    this->omodes |= modes;
}
void olstream::setStream(const char* file) {
    this->fmode = 1;
    
    if (file) {
        delete[] this->fname;
        this->fname = new char[strlen(file)+1];
        strcpy(this->fname, file);
    }
}
void olstream::setStream(const char* file, const OMODE modes) {
    this->fmode = 1;
    this->omodes = modes;
    if (file) {
        delete[] this->fname;
        this->fname = new char[strlen(file)+1];
        strcpy(this->fname, file);
    }
}
void olstream::setStream(std::ostream* stream) {
    this->fmode = 0;
    this->stream = stream;
}

std::ostream& olstream::getStream() {
    return streamGuard();
}
const char* olstream::getPath() {
    return this->fname;
}

std::ostream& olstream::open() {
    if (this->fname) {
        if (!this->file.is_open()) {
            this->file.open(this->fname);
        }
        return this->file;
    }
    return streamGuard();
}
void olstream::close() {
    this->file.close();
}

/*void olstream::test() {
    std::cout << '\t' << fmode << newline;
    std::cout << '\t' << (bool)file << newline;;
    std::cout << '\t' << stream << newline;
    std::cout << '\t' << fname << newline;
    std::cout << '\t' << omodes << newline;
}*/



lstream::lstream() : 
    basic_fstream<char>() {}	//default
lstream::lstream(const char* file, const OMODE modes) :	//file stream mode
    basic_fstream<char>(), file(file), modes(modes) {}
lstream::lstream(const char* file, void* init, const OMODE modes) :	//file stream mode, open by default
    basic_fstream<char>(file, modes), file(file), modes(modes) {}
lstream::lstream(const lstream& other) : 
    basic_fstream<char>(), file(other.file), modes(other.modes) {}
lstream::lstream(lstream&& other) : 
    basic_fstream<char>(static_cast<basic_fstream<char>&&>(other)), file(other.file), modes(other.modes) {}

void lstream::operator=(const lstream& other) {
    this->file = other.file;
    this->modes = other.modes;
}
void lstream::operator=(lstream&& other) {
    *(static_cast<basic_fstream<char>*>(this)) = std::move(static_cast<basic_fstream<char>&&>(other));
    this->file = other.file;
    this->modes = other.modes;
}

void lstream::setFile(const char* file, OMODE modes) {
    this->file = file;
    this->modes = modes;
}
void lstream::setMode(OMODE modes) {
    this->modes = modes;
}
void lstream::addMode(OMODE modes) {
    this->modes |= modes;
}
const char* lstream::getFile() { return file; }
OMODE lstream::getModes() { return modes; }

void lstream::openFile(const char* file, OMODE modes) {
    if (this->is_open()) {
        close();
    }
    this->file = file;
    this->modes = modes;
    open(file, modes);
}
#undef OMODE
void lstream::openInput() {
    if (this->is_open()) {
        close();
    }
    open(file, ((modes | std::ios::in) & ~(std::ios::out | std::ios::trunc)));
}
void lstream::openOutput() {
    if (this->is_open()) {
        close();
    }
    open(file, ((modes | std::ios::out) & ~std::ios::in));
}