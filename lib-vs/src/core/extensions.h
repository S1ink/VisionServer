#pragma once

#include <array>
#include <type_traits>

#include "opencv2/opencv.hpp"


namespace vs2 {

enum class BGR {
	BLUE = 0,
	GREEN = 1,
	RED = 2
};

template<typename t = uint16_t>
inline t operator~(BGR v) {
	static_assert(std::is_arithmetic<t>::value, "Template paramter (t) must be arithmetic type.");
	return static_cast<t>(v);
}

inline static std::array<std::array<uint16_t, 2>, 3> weights_map{
	std::array<uint16_t, 2>{~BGR::GREEN, ~BGR::RED},		// blue
	std::array<uint16_t, 2>{~BGR::BLUE, ~BGR::RED},		// green
	std::array<uint16_t, 2>{~BGR::BLUE, ~BGR::GREEN},	// red
};



} // namespace vs2


inline static const std::array<std::array<cv::Scalar, 3>, 3> markup_map{
    std::array<cv::Scalar, 3>{cv::Scalar(255, 0, 0), cv::Scalar(255, 127, 0), cv::Scalar(255, 255, 0)},	//blue
	std::array<cv::Scalar, 3>{cv::Scalar(0, 255, 0), cv::Scalar(0, 255, 127), cv::Scalar(0, 255, 255)},	//green
	std::array<cv::Scalar, 3>{cv::Scalar(0, 0, 255), cv::Scalar(127, 0, 255), cv::Scalar(255, 0, 255)},	//red
};