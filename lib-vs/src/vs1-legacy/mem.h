#pragma once

#include <stdint.h>

extern "C" void memcpy_threshold_asm(uint8_t* dest, const uint8_t* src, int size, int minimum);
extern "C" void memcpy_threshold_binary_asm(uint8_t* dest, const uint8_t* src, int size, int minimum);
extern "C" void memcpy_subtract_asm(uint8_t* base, uint8_t* sub, uint8_t* dest, int size);