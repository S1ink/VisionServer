#pragma once

#include <stdint.h>


#ifndef ENABLE_WRAPPER_DIM_SAFETY
#define ENABLE_WRAPPER_DIM_SAFETY true		// use CV_Assert for checking correnct mat sizes/channel nums
#endif

#ifdef __cplusplus
extern "C" {
#endif

void memcpy_threshold_asm(	// keeps original threshold values
	uint8_t const *src,
	uint8_t* dest,
	int32_t size,
	uint8_t thresh
);
void memcpy_threshold_binary_asm(	// thresholds evaulate to all 1's rather than their original value
	uint8_t const *src,
	uint8_t* dest,
	int32_t size,
	uint8_t thresh
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
void memcpy_deinterlace_wstb_asm(		// subtracts weighted channels from primary, adds gamma, and thresholds into binary frame
	uint8_t const *framebuff_3C,
	uint8_t *dest,
	int32_t size,
	uint8_t offset,
	uint8_t alpha,
	uint8_t beta,
	uint8_t gamma,
	uint8_t thresh
);
void memcpy_deinterlace_togray_asm(
	uint8_t const *framebuff_3C,
	uint8_t *dest,
	int32_t size
);

void memcpy_bitwise_or_asm(
	uint8_t const *a,
	uint8_t const *b,
	uint8_t* dest,
	int32_t size
);
void memcpy_bitwise_or_3c_asm(	// computes the bitwise or of a binary frame with each channel of a 3-channel frame
	uint8_t const *a_3C,
	uint8_t const *b,
	uint8_t* dest_3C,
	int32_t size
);
void memcpy_add_asm(		// totally unnecessary but it wouldn't be 'neon_add' w/o the definite function
	uint8_t const *a,
	uint8_t const *b,
	uint8_t* dest,
	int32_t size
);
void memcpy_add_3c_asm(		// for combining a 3-channel image w/ a single channel binary image w/o the need for pre-converting
	uint8_t const *a_3C,
	uint8_t const *b,
	uint8_t* dest_3C,
	int32_t size
);
void memcpy_add_3c2_asm(
	uint8_t const *a_3C,
	uint8_t const *b,
	uint8_t* dest_3C,
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

inline void neon_threshold(
	const cv::Mat& src, cv::Mat& dest, uint8_t thresh
) {
#if ENABLE_WRAPPER_DIM_SAFETY
	CV_Assert(src.type() == CV_8UC1);
	CV_Assert(dest.type() == CV_8UC1);
	CV_Assert(src.size().area() == dest.size().area());
#endif
	memcpy_threshold_asm(
		src.data,
		dest.data,
		src.size().area(),
		thresh
	);
}
inline void neon_threshold_binary(
	const cv::Mat& src, cv::Mat& dest, uint8_t thresh
) {
#if ENABLE_WRAPPER_DIM_SAFETY
	CV_Assert(src.type() == CV_8UC1);
	CV_Assert(dest.type() == CV_8UC1);
	CV_Assert(src.size().area() == dest.size().area());
#endif
	memcpy_threshold_binary_asm(
		src.data,
		dest.data,
		src.size().area(),
		thresh
	);
}
inline void neon_compare3_add(
	const cv::Mat& primary,
	const cv::Mat& comp1, const cv::Mat& comp2,
	const cv::Mat& add, cv::Mat& dest
) {
#if ENABLE_WRAPPER_DIM_SAFETY
	CV_Assert(primary.type() == CV_8UC1);
	CV_Assert(comp1.type() == CV_8UC1);
	CV_Assert(comp2.type() == CV_8UC1);
	CV_Assert(add.type() == CV_8UC1);
	CV_Assert(dest.type() == CV_8UC1);
	CV_Assert(primary.size().area() == comp1.size().area());
	CV_Assert(primary.size().area() == comp2.size().area());
	CV_Assert(primary.size().area() == add.size().area());
	CV_Assert(primary.size().area() == dest.size().area());
#endif
	memcpy_compare3_add_asm(
		primary.data,
		comp1.data,
		comp2.data,
		add.data,
		dest.data,
		primary.size().area()
	);
}
inline void neon_wst(
	const cv::Mat& primary,
	const cv::Mat& ch1, uint8_t alpha,
	const cv::Mat& ch2, uint8_t beta,
	cv::Mat& dest, uint8_t gamma = 0
) {
#if ENABLE_WRAPPER_DIM_SAFETY
	CV_Assert(primary.type() == CV_8UC1);
	CV_Assert(ch1.type() == CV_8UC1);
	CV_Assert(ch2.type() == CV_8UC1);
	CV_Assert(dest.type() == CV_8UC1);
	CV_Assert(primary.size().area() == ch1.size().area());
	CV_Assert(primary.size().area() == ch2.size().area());
	CV_Assert(primary.size().area() == dest.size().area());
#endif
	memcpy_wst_asm(
		primary.data,
		ch1.data,
		ch2.data,
		dest.data,
		primary.size().area(),
		alpha, beta, gamma
	);
}
inline void neon_deinterlace_wstb(
	const cv::Mat& frame_3C, cv::Mat& dest,
	vs2::BGR primary, uint8_t alpha = 0xFF, uint8_t beta = 0xFF, uint8_t gamma = 0, uint8_t thresh = 0x7F
) {
#if ENABLE_WRAPPER_DIM_SAFETY
	CV_Assert(frame_3C.type() == CV_8UC3);
	CV_Assert(dest.type() == CV_8UC1);
	CV_Assert(frame_3C.size().area() == dest.size().area());
#endif
	memcpy_deinterlace_wstb_asm(
		frame_3C.data,
		dest.data,
		frame_3C.size().area(),
		~primary, alpha, beta, gamma, thresh);
}
inline void neon_deinterlace_cvt2gray(
	const cv::Mat& frame_3C, cv::Mat& dest
) {
#if ENABLE_WRAPPER_DIM_SAFETY
	CV_Assert(frame_3C.type() == CV_8UC3);
	CV_Assert(dest.type() == CV_8UC1);
	CV_Assert(frame_3C.size().area() == dest.size().area());
#endif
	memcpy_deinterlace_togray_asm(
		frame_3C.data,
		dest.data,
		frame_3C.size().area()
	);
}

inline void neon_bitwise_or(
	const cv::Mat& a, const cv::Mat& b, cv::Mat& dest
) {
#if ENABLE_WRAPPER_DIM_SAFETY
	CV_Assert(a.size().area() == b.size().area());
	CV_Assert(a.size().area() == dest.size().area());
#endif
	if(a.type() == b.type()) {
#if ENABLE_WRAPPER_DIM_SAFETY
	CV_Assert(a.type() == dest.type());
#endif
		memcpy_bitwise_or_asm(
			a.data,
			b.data,
			dest.data,
			a.size().area() * a.channels()
		);
	} else if(a.type() == CV_8UC3) {
#if ENABLE_WRAPPER_DIM_SAFETY
	CV_Assert(b.type() == CV_8UC1);
	CV_Assert(dest.type() == CV_8UC3);
#endif
		memcpy_bitwise_or_3c_asm(
			a.data,
			b.data,
			dest.data,
			a.size().area()
		);
	} else {
#if ENABLE_WRAPPER_DIM_SAFETY
	CV_Assert(a.type() == CV_8UC1);
	CV_Assert(b.type() == CV_8UC3);
	CV_Assert(dest.type() == CV_8UC3);
#endif
		memcpy_bitwise_or_3c_asm(
			b.data,
			a.data,
			dest.data,
			a.size().area()
		);
	}
}
inline void neon_add(
	const cv::Mat& a, const cv::Mat& b, cv::Mat& dest
) {
#if ENABLE_WRAPPER_DIM_SAFETY
	CV_Assert(a.size().area() == b.size().area());
	CV_Assert(a.size().area() == dest.size().area());
#endif
	if(a.type() == CV_8UC3) {
#if ENABLE_WRAPPER_DIM_SAFETY
		CV_Assert(dest.type() == CV_8UC3);
#endif
		if(b.type() == CV_8UC3) {
			memcpy_add_3c2_asm(
				a.data,
				b.data,
				dest.data,
				a.size().area()
			);
		} else {
#if ENABLE_WRAPPER_DIM_SAFETY
			CV_Assert(b.type() == CV_8UC1);
#endif
			memcpy_add_3c_asm(
				a.data,
				b.data,
				dest.data,
				a.size().area()
			);
		}
	} else if(b.type() == CV_8UC3) {
#if ENABLE_WRAPPER_DIM_SAFETY
	CV_Assert(a.type() == CV_8UC1);
	CV_Assert(dest.type() == CV_8UC3);
#endif
		memcpy_add_3c_asm(
			b.data,
			a.data,
			dest.data,
			a.size().area()
		);
	} else {
#if ENABLE_WRAPPER_DIM_SAFETY
	CV_Assert(a.type() == CV_8UC1);
	CV_Assert(b.type() == CV_8UC1);
	CV_Assert(dest.type() == CV_8UC1);
#endif
		memcpy_add_asm(
			a.data,
			b.data,
			dest.data,
			a.size().area()
		);
	}
}
inline void neon_subtract(
	const cv::Mat& src, const cv::Mat& sub, cv::Mat& dest
) {
#if ENABLE_WRAPPER_DIM_SAFETY
	CV_Assert(src.type() == CV_8UC1);
	CV_Assert(sub.type() == CV_8UC1);
	CV_Assert(dest.type() == dest.type());
	CV_Assert(src.size().area() == sub.size().area());
	CV_Assert(src.size().area() == dest.size().area());
#endif
	memcpy_subtract_asm(
		src.data,
		sub.data,
		dest.data,
		src.size().area()
	);
}