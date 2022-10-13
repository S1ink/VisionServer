#pragma once

#include <stdint.h>

extern "C" void memcpy_threshold_asm(	// keeps original threshold values
	uint8_t* dest,
	uint8_t const *src,
	int32_t size, int32_t minimum
);
extern "C" void memcpy_threshold_binary_asm(	// thresholds evaulate to all 1's rather than their original value
	uint8_t* dest,
	uint8_t const *src,
	int32_t size, int32_t minimum
);
extern "C" void memcpy_compare3_add_asm(	// threshold a channel based on if it has the highest value of all (3), then add some other amount (WST result)
	uint8_t const *primary,
	uint8_t const *cmp1,
	uint8_t const *cmp2,
	uint8_t const *add,
	uint8_t* dest,
	int32_t size
);
extern "C" void memcpy_bitwise_or_asm(
	uint8_t const *a,
	uint8_t const *b,
	uint8_t* dest,
	int32_t size
);

extern "C" void memcpy_subtract_asm(	// don't use this it does not account for underflows
	uint8_t const *base,
	uint8_t const *sub,
	uint8_t* dest,
	int32_t size
);