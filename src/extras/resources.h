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

#ifdef _WIN32
#define WINDOWS 1
#endif
#ifdef __linux__
#define LINUX 1
#endif
#ifdef __APPLE__
#define MACOS 1
#endif

template <template <typename...> class C, typename...Ts>
std::true_type is_base_of_template_impl(const C<Ts...>*);
template <template <typename...> class C>
std::false_type is_base_of_template_impl(...);
template <typename T, template <typename...> class C>
using is_base_of_template = decltype(is_base_of_template_impl<C>(std::declval<T*>()));

template <template <size_t> class C, size_t S>
std::true_type is_base_of_num_template_impl(const C<S>*);
template <template <size_t...> class C>
std::false_type is_base_of_num_template_impl(...);
template <typename T, template <size_t> class C>
using is_base_of_num_template = decltype(is_base_of_num_template_impl<C>(std::declval<T*>()));

//#endif