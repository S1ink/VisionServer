#pragma once

#include <vector>
#include <array>
#include <string>
#include <unordered_map>

#include <opencv2/core/types.hpp>
#include <wpi/json.h>


#ifndef CALIB_DEFAULT_FLOAT_TYPE
#define CALIB_DEFAULT_FLOAT_TYPE	float
#endif
#ifndef CALIB_DEFAULT_STRING_TYPE
#define CALIB_DEFAULT_STRING_TYPE	std::string
#endif
#ifndef CALIB_DEFAULT_MAP_TYPE
#define CALIB_DEFAULT_MAP_TYPE		std::unordered_map
#endif

template<typename... Ts>
using VecMap = std::vector<std::pair<Ts...> >;


template<
	typename fp_T = CALIB_DEFAULT_FLOAT_TYPE>
using Calibration_ =
	std::array<cv::Mat_<fp_T>, 2>;
typedef Calibration_<>	Calibration;

template<
	typename fp_T = CALIB_DEFAULT_FLOAT_TYPE,
	template<class...> typename map_T = CALIB_DEFAULT_MAP_TYPE>
using CalibSet_ =
	map_T<cv::Size2i, Calibration_<fp_T> >;
typedef CalibSet_<>		CalibSet;

template<
	typename fp_T = CALIB_DEFAULT_FLOAT_TYPE,
	typename str_T = CALIB_DEFAULT_STRING_TYPE,
	template<class...> typename map_T = CALIB_DEFAULT_MAP_TYPE>
using CalibMap_ =
	map_T<str_T, CalibSet_<fp_T, map_T> >;
typedef CalibMap_<>		CalibMap;

template<
	typename fp_T = CALIB_DEFAULT_FLOAT_TYPE,
	typename str_T = CALIB_DEFAULT_STRING_TYPE>
using CalibList_	=	CalibMap_<fp_T, str_T, VecMap>;
typedef CalibList_<>	CalibList;



/* Searches for calibrations given the following json layout:
	"calibrations" : {
		"<camera type>" : {
			"<WIDTH>x<HEIGHT>" : {
				"camera_matrix" : [ [x3], [x3], [x3] ],
				"distortion" : [ [x5] ]
			},
			...
		},
		...
	}
*/
// template<
// 	typename fp_T = CALIB_DEFAULT_FLOAT_TYPE,
// 	typename str_T = CALIB_DEFAULT_STRING_TYPE,
// 	template<class...> typename map_T = CALIB_DEFAULT_MAP_TYPE>
// int loadJsonCalibs(const wpi::json& src, CalibMap_<fp_T, str_T, map_T>& calibs) {
// 	using CalibMap_T = CalibMap_<fp_T, str_T, map_T>;
// 	using CalibSet_T = CalibSet_<fp_T, map_T>;
// 	using Calib_T = Calibration_<fp_T>;
// 	// test map implementation qualifiers
// 	int count = -1;
// 	if((count = src.count("calibrations")) > 0) {
// 		for(auto& calib : src.at("calibrations")) {

// 		}
// 	}
// 	return count;
// }

// template<typename num_T = int>
// cv::Size_<num_T> extractSize(std::string_view str) {
// `	size_t a = str.find('x');
// }