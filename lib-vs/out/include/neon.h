#pragma once

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

void memcpy_threshold_asm(	// keeps original threshold values
	uint8_t* dest,
	uint8_t const *src,
	int32_t size, uint8_t minimum
);
void memcpy_threshold_binary_asm(	// thresholds evaulate to all 1's rather than their original value
	uint8_t* dest,
	uint8_t const *src,
	int32_t size, uint8_t minimum
);
void memcpy_compare3_add_asm(	// threshold a channel based on if it has the highest value of all (3), then add some other amount (WST result)
	uint8_t const *primary,
	uint8_t const *cmp1,
	uint8_t const *cmp2,
	uint8_t const *add,
	uint8_t* dest,
	int32_t size
);
void memcpy_wst_asm(
	uint8_t const *primary,
	uint8_t const *ch2,
	uint8_t const *ch3,
	uint8_t *dest,
	int32_t size,
	uint8_t alpha,
	uint8_t beta,
	uint8_t gamma
);
void memcpy_split_wst_asm(
	uint8_t const *framebuff_3C,
	uint8_t *dest,
	int32_t size,
	uint8_t offset,
	uint8_t alpha,
	uint8_t beta,
	uint8_t gamma
);

void memcpy_bitwise_or_asm(
	uint8_t const *a,
	uint8_t const *b,
	uint8_t* dest,
	int32_t size
);
void memcpy_subtract_asm(	// don't use this it does not account for underflows (10/29/22)--> maybe fixed now?
	uint8_t const *base,
	uint8_t const *sub,
	uint8_t* dest,
	int32_t size
);

#ifdef __cplusplus
}
#endif




#include <opencv2/opencv.hpp>
#include "extensions.h"

inline void neon_threshold(cv::InputArray src, cv::OutputArray dest, uint8_t thresh)
	{ memcpy_threshold_asm(dest.getMatRef().data, src.getMat().data, src.getSz().area(), thresh); }
inline void neon_threshold_binary(cv::InputArray src, cv::OutputArray dest, uint8_t thresh)
	{ memcpy_threshold_binary_asm(dest.getMatRef().data, src.getMat().data, src.getSz().area(), thresh); }
inline void neon_compare3_add(cv::InputArray primary, cv::InputArray comp1, cv::InputArray comp2, cv::InputArray add, cv::OutputArray dest)
	{ memcpy_compare3_add_asm(primary.getMat().data, comp1.getMat().data, comp2.getMat().data, add.getMat().data, dest.getMatRef().data, primary.getSz().area()); }
inline void neon_wst(cv::InputArray primary, cv::InputArray ch1, uint8_t alpha, cv::InputArray ch2, uint8_t beta, cv::OutputArray dest, uint8_t gamma = 0)
	{ memcpy_wst_asm(primary.getMat().data, ch1.getMat().data, ch2.getMat().data, dest.getMatRef().data, primary.getSz().area(), alpha, beta, gamma); }
inline void neon_split_wst(cv::InputArray frame_3C, cv::OutputArray dest, vs2::BGR primary, uint8_t alpha, uint8_t beta, uint8_t gamma = 0)
	{ memcpy_split_wst_asm(frame_3C.getMat().data, dest.getMatRef().data, frame_3C.getSz().area(), ~primary, alpha, beta, gamma); }

inline void neon_bitwise_or(cv::InputArray a, cv::InputArray b, cv::OutputArray dest)
	{ memcpy_bitwise_or_asm(a.getMat().data, b.getMat().data, dest.getMatRef().data, a.getSz().area()); }
inline void neon_subtract(cv::InputArray src, cv::InputArray sub, cv::OutputArray dest)
	{ memcpy_subtract_asm(src.getMat().data, sub.getMat().data, dest.getMatRef().data, src.getSz().area()); }