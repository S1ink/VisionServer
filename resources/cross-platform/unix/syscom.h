#pragma once

#ifdef __unix__

#include <iostream>
#include <sstream>

#include "../resources.h"

void exec(const char* command, std::ostream& output);

int aptUpdate();
int aptUpdate(std::ostream& out);
int aptUpgrade();
int aptUpgrade(std::ostream& out);

void rsync(std::ostream& output, const char* source, const char* destination, const char* options = "-va");
void rclone(std::ostream& output, const char* source, const char* destination, const char* mode = "sync");
void rsync(const char* source, const char* destination, const char* options = "-a");
void rclone(const char* source, const char* destination, const char* mode = "sync");

#else 
#pragma message "This code only works on unix (for now), including will only increase binary size!"
#endif