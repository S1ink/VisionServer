#pragma once

#include <vector>
#include <array>
#include <string>
#include <map>
#include <unordered_map>

#include <opencv2/core/types.hpp>
#include <wpi/json.h>
#include <wpi/StringMap.h>


template<typename first_T, typename second_T>
using VecMap = std::vector<std::pair<first_T, second_T> >;

#ifndef CALIB_DEFAULT_FLOAT_TYPE
#define CALIB_DEFAULT_FLOAT_TYPE	float
#endif
#ifndef CALIB_DEFAULT_STRING_TYPE
#define CALIB_DEFAULT_STRING_TYPE	std::string
#endif
#ifndef CALIB_DEFAULT_MAP_TYPE
#define CALIB_DEFAULT_MAP_TYPE		VecMap
#endif
#ifndef CALIB_DEFAULT_SMAP_TYPE
#define CALIB_DEFAULT_SMAP_TYPE		wpi::StringMap
#endif

template<
	typename str_T = CALIB_DEFAULT_STRING_TYPE,
	template<typename...> typename map_T = CALIB_DEFAULT_MAP_TYPE>
struct StringMapGen {
	template<typename V, typename S = str_T>
	using type = map_T<S, V>;
};
template<typename str_T = CALIB_DEFAULT_STRING_TYPE>
using VecSMap_ = StringMapGen<str_T, VecMap>::type;
typedef VecSMap_<>	VecSMap;
template<typename str_T = CALIB_DEFAULT_STRING_TYPE>
using StdSMap_ = StringMapGen<str_T, std::map>::type;
typedef StdSMap_<>	StdSMap;
template<typename str_T = CALIB_DEFAULT_STRING_TYPE>
using StdSUmap_ = StringMapGen<str_T, std::unordered_map>::type;
typedef StdSUmap_<>	StdSUmap;


template<
	typename fp_T = CALIB_DEFAULT_FLOAT_TYPE>
using Calibration_ =
	std::array<cv::Mat_<fp_T>, 2>;		// a camera matx and dist coeff matx pair
typedef Calibration_<>	Calibration;

template<
	typename fp_T = CALIB_DEFAULT_FLOAT_TYPE,
	template<class...> typename map_T = CALIB_DEFAULT_MAP_TYPE>
using CalibSet_ =
	map_T<cv::Size2i, Calibration_<fp_T> >;		// map of resolution <--> calibration
typedef CalibSet_<>		CalibSet;

template<
	typename fp_T = CALIB_DEFAULT_FLOAT_TYPE,
	template<typename...> typename cmap_T = CALIB_DEFAULT_MAP_TYPE,
	template<typename...> typename smap_T = CALIB_DEFAULT_SMAP_TYPE>
using CalibSMap_ =
	smap_T<CalibSet_<fp_T, cmap_T> >;		// stringmap of calibration sets
typedef CalibSMap_<>	CalibSMap;

template<
	typename fp_T = CALIB_DEFAULT_FLOAT_TYPE,
	typename str_T = CALIB_DEFAULT_FLOAT_TYPE,
	template<typename...> typename cmap_T = CALIB_DEFAULT_MAP_TYPE,
	template<typename...> typename omap_T = CALIB_DEFAULT_MAP_TYPE>
using CalibMat_ =
	CalibSMap_<fp_T, cmap_T, StringMapGen<str_T, omap_T>::type>;	// integrated stringmap generation
typedef CalibMat_<>		CalibMat;

template<
	typename fp_T = CALIB_DEFAULT_FLOAT_TYPE,
	typename str_T = CALIB_DEFAULT_STRING_TYPE,
	template<class...> typename map_T = CALIB_DEFAULT_MAP_TYPE>
using CalibMap_ =
	CalibMat_<fp_T, str_T, map_T, map_T>;		// ^ but the same map type for inner and outer
typedef CalibMap_<>		CalibMap;

template<typename fp_T = CALIB_DEFAULT_FLOAT_TYPE>
using CalibSList_ = CalibSMap_<fp_T, str_T, VecMap>;
typedef CalibSList_<>	CalibSList;

template<
	typename fp_T = CALIB_DEFAULT_FLOAT_TYPE,
	typename str_T = CALIB_DEFAULT_STRING_TYPE>
using CalibList_ = CalibMap_<fp_T, str_T, VecMap>;
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

#include <charconv>
template<typename num_T = int>
cv::Size_<num_T> extractSize(std::string_view str) {
	cv::Size_<num_T> ret;
	size_t a = str.find('x');
	std::from_chars<num_T>(str.data(), str.data() + a, std::ref(ret.width));
	std::from_chars<num_T>(str.data() + a + 1, str.data() + str.length(), std::ref(ret.height));
	return ret;
}


#include <type_traits>
#include <cpp-tools/src/types.h>
template<
	typename fp_T,
	template<typename...> typename cmap_T,
	template<typename...> typename smap_T>
struct CALIB {
	// static assertion
	static_assert(std::is_floating_point<fp_T>::value, "Template param error: fp_T must be floating point type.");

	using Calib_T = Calibration_<fp_T>;
	using CalibSet_T = CalibSet_<fp_T, cmap_T>;
	using CalibMap_T = CalibSMap_<fp_T, cmap_T, smap_T>;

	static constexpr inline bool
		is_wpi_smap = is_same_template<smap_T, wpi::StringMap>::value,
		is_seq_smap = is_same_template<smap_T, VecMap>::value,		// this needs to be generalized for any or the correct string type param
		is_assoc_smap = is_same_template<smap_T, std::map>::value ||
			is_same_template<smap_T, std::unordered_map>::value,	// ^
		is_seq_cmap = is_same_template<cmap_T, VecMap>::value,
		is_assoc_cmap = is_same_template<cmap_T, std::map>::value ||
			is_same_template<cmap_T, std::unordered_map>::value
	;

	// static int loadJsonBlock(const wpi::json& blk, CalibMap_T& calibs) {
	// 	for(auto& calib : blk) {
	// 		CalibSet_T set;
	// 	}
	// }

	static CalibSet_T* findSet(std::string_view name, const CalibMap_T& map) {

	}

	static int search(
		std::string_view c, const cv::Size2i& sz, const CalibMap_T& map,
		cv::Mat1f& matx, cv::Mat1f& coeff
	) {
		if constexpr(is_seq_container) {
			return seq_search_impl(c, sz, map, matx, coeff);
		} else if constexpr(is_assoc_container) {
			return assoc_search_impl(c, sz, map, matx, coeff);
		} else {

		}
	}

private:
	static CalibSet_T* wpi_find_set_impl(std::string_view name, const CalibMap_T& map) {

	}
	static CalibSet_T* seq_find_set_impl(std::string_view name, const CalibMap_T& map) {

	}
	static CalibSet_T* assoc_find_set_impl(std::string_view name, const CalibMap_T& map) {

	}

	static int seq_search_impl(
		std::string_view c, const cv::Size2i& sz, const CalibMap_T& map,
		cv::Mat1f& matx, cv::Mat1f& coeff
	) {

	}
	static int assoc_search_impl(
		std::string_view c, const cv::Size2i& sz, const CalibMap_T& map,
		cv::Mat1f& matx, cv::Mat1f& coeff
	) {
		
	}

};