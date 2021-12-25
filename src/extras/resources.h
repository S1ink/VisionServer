#pragma once

//#ifndef _RESOURCES
//#define _RESOURCES

#define CHRONO std::chrono
#define CE_STR constexpr char const*

constexpr char newline = 10;	// ('\n')
constexpr char space = 32;		// (' ')
constexpr char comma = 44;		// (',')
constexpr char cr = 13;			// ('\r' or carriage-return)
constexpr char null = 0;		// ('\0')
constexpr char s_dir = 
#ifdef _WIN32
92;                             //('\')
#else //if '__unix__' or '__linux__' or '__APPLE__' 
47;                             //('/')
#endif

CE_STR endline = 
#ifdef _WIN32
"\r\n";
#else 
"\n";
#endif

// #ifdef _WIN32
// #define WINDOWS 1
// #endif
// #ifdef __linux__
// #define LINUX 1
// #endif
// #ifdef __APPLE__
// #define MACOS 1
// #endif

//#endif