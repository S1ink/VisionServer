#include "syscom.h"

void exec(const char* command, std::ostream& output) {
    FILE* pipe = popen(command, "r");
    if (!pipe) {
        output << "popen function failed";
    }
    else {
        char buffer[128];
        while (fgets(buffer, 128, pipe) != NULL) {
            output << buffer;
        }
    }
    pclose(pipe);
}

int aptUpdate() {
    int result = 0;
    FILE* pipe = popen("sudo apt update", "r");
    if (!pipe) {
        result = -1;
    }
    else {
        char buffer[128];
        std::string line;
        while (fgets(buffer, 128, pipe)) {
            line.append(buffer);
            if (std::stringstream(line) >> result) {
                break;
            }
            else {
                line.clear();
            }
        }
    }
    pclose(pipe);
    return result;
}
int aptUpdate(std::ostream& out) {
    int result = 0;
    FILE* pipe = popen("sudo apt update", "r");
    if (!pipe) {
        result = -1;
    }
    else {
        char buffer[128];
        std::string line;
        bool found = false;
        while (fgets(buffer, 128, pipe)) {
            out << buffer;
            if (!found) {
                line.append(buffer);
                if (std::stringstream(line) >> result) {
                    found = true;
                }
                else {
                    line.clear();
                }
            }
        }
    }
    pclose(pipe);
    return result;
}
int aptUpgrade() {
    int result = 0;
    FILE* pipe = popen("sudo apt-get upgrade -y", "r");
    if (!pipe) {
        result = -1;
    }
    else {
        char buffer[128];
        std::string line;
        while (fgets(buffer, 128, pipe)) {
            line.append(buffer);
            if (std::stringstream(line) >> result) {
                break;
            }
            else {
                line.clear();
            }
        }
    }
    pclose(pipe);
    return result;
}
int aptUpgrade(std::ostream& out) {
    int result = 0;
    FILE* pipe = popen("sudo apt-get upgrade -y", "r");
    if (!pipe) {
        result = -1;
    }
    else {
        char buffer[128];
        std::string line;
        bool found = false;
        while (fgets(buffer, 128, pipe)) {
            out << buffer;
            if (!found) {
                line.append(buffer);
                if (std::stringstream(line) >> result) {
                    found = true;
                }
                else {
                    line.clear();
                }
            }
        }
    }
    pclose(pipe);
    return result;
}

//add output parser because rsync default is horrendous
void rsync(std::ostream& output, const char* source, const char* destination, const char* options) {
    std::ostringstream command;
    command << "sudo rsync " << options << space << source << space << destination;
    FILE* pipe = popen(command.str().c_str(), "r");
    if (!pipe) {
        output << "popen function failed";
    }
    else {
        char buffer[128];
        while (fgets(buffer, 128, pipe) != NULL) {
            output << buffer;
        }
    }
    pclose(pipe);
}
void rclone(std::ostream& output, const char* source, const char* destination, const char* mode) {
    std::ostringstream command;
    command << "rclone " << mode << space << source << space << destination << " -vv";
    FILE* pipe = popen(command.str().c_str(), "r");
    if (!pipe) {
        output << "popen function failed";
    }
    else {
        char buffer[128];
        while (fgets(buffer, 128, pipe) != NULL) {
            output << buffer;
        }
    }
    pclose(pipe);
}
void rsync(const char* source, const char* destination, const char* options) {
    std::ostringstream command;
    command << "sudo rsync " << options << space << source << space << destination;
    system(command.str().c_str());
}
void rclone(const char* source, const char* destination, const char* mode) {
    std::ostringstream command;
    command << "rclone " << mode << space << source << space << destination;
    system(command.str().c_str());
}