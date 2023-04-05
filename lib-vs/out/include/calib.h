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

// template<
// 	typename str_T = CALIB_DEFAULT_STRING_TYPE,
// 	template<typename...> typename map_T = CALIB_DEFAULT_MAP_TYPE>
// struct StringMapGen {
// 	template<typename V, typename S = str_T>
// 	using type = map_T<S, V>;
// };
// template<typename str_T = CALIB_DEFAULT_STRING_TYPE>
// using VecSMap_ = StringMapGen<str_T, VecMap>::type;
// typedef VecSMap_<>	VecSMap;
// template<typename str_T = CALIB_DEFAULT_STRING_TYPE>
// using StdSMap_ = StringMapGen<str_T, std::map>::type;
// typedef StdSMap_<>	StdSMap;
// template<typename str_T = CALIB_DEFAULT_STRING_TYPE>
// using StdSUmap_ = StringMapGen<str_T, std::unordered_map>::type;
// typedef StdSUmap_<>	StdSUmap;


template<
	typename fp_T = CALIB_DEFAULT_FLOAT_TYPE>
using Calibration_ =
std::array<cv::Mat_<fp_T>, 2>;		// a camera matx and dist coeff matx pair
typedef Calibration_<>	Calibration;

struct __CalibSet { protected: __CalibSet() {} };
template<
	typename fp_T = CALIB_DEFAULT_FLOAT_TYPE,
	template<class, class, class...> typename map_T = CALIB_DEFAULT_MAP_TYPE>
struct CalibSet_ : public map_T<cv::Size2i, Calibration_<fp_T> >, __CalibSet {		// map of resolution <--> calibration
	using map_T<cv::Size2i, Calibration_<fp_T> >::map_T;

	template<
		typename F,
		template<class, class, class...> typename M>
	using Self = CalibSet_<fp_T, map_T>;

	using Cal_T = Calibration_<fp_T>;
	using Set_T = map_T<cv::Size2i, Calibration_<fp_T> >;
	typedef Set_T Super;

	using Float_T = fp_T;
	template<class A, class B>
	using CalMap_T = map_T<A, B>;
};
typedef CalibSet_<>		CalibSet;

struct __CalibMap { protected: __CalibMap() {} };
struct __CalibSMap : __CalibMap { protected: __CalibSMap() {} };
template<
	typename fp_T = CALIB_DEFAULT_FLOAT_TYPE,
	template<class, class, class...> typename cmap_T = CALIB_DEFAULT_MAP_TYPE,
	template<class, class...> typename smap_T = CALIB_DEFAULT_SMAP_TYPE>
struct CalibSMap_ : public smap_T<CalibSet_<fp_T, cmap_T> >, __CalibSMap {		// stringmap of calibration sets
	using smap_T<CalibSet_<fp_T, cmap_T> >::smap_T;

	template<
		typename F,
		template<class, class, class...> typename C,
		template<class, class...> typename S>
	using Self = CalibSMap_<F, C, S>;

	using Cal_T = Calibration_<fp_T>;
	using Set_T = CalibSet_<fp_T, cmap_T>;
	using Map_T = smap_T<CalibSet_<fp_T, cmap_T> >;
	typedef Map_T Super;

	using Float_T = fp_T;
	template<class A, class B>
	using CalMap_T = cmap_T<A, B>;
	template<class A>
	using StrMap_T = smap_T<A>;
};
typedef CalibSMap_<>	CalibSMap;

struct __CalibMat : __CalibMap { protected: __CalibMat() {} };
template<
	typename fp_T = CALIB_DEFAULT_FLOAT_TYPE,
	typename str_T = CALIB_DEFAULT_STRING_TYPE,
	template<class, class, class...> typename cmap_T = CALIB_DEFAULT_MAP_TYPE,
	template<class, class, class...> typename nmap_T = CALIB_DEFAULT_MAP_TYPE>
struct CalibMat_ : public nmap_T<str_T, CalibSet_<fp_T, cmap_T> >, __CalibMat {		// normal outer map type with string key type
	using nmap_T<str_T, CalibSet_<fp_T, cmap_T> >::nmap_T;

	template<
		typename F, typename S,
		template<class, class, class...> typename C,
		template<class, class...> typename N>
	using Self = CalibMat_<F, S, C, N>;

	using Cal_T = Calibration_<fp_T>;
	using Set_T = CalibSet_<fp_T, cmap_T>;
	using Map_T = nmap_T<str_T, CalibSet_<fp_T, cmap_T> >;
	typedef Map_T Super;

	using Float_T = fp_T;
	using String_T = str_T;
	template<class A, class B>
	using CalMap_T = cmap_T<A, B>;
	template<class A, class B>
	using NameMap_T = nmap_T<A, B>;
};
typedef CalibMat_<>		CalibMat;

// template<
// 	typename fp_T = CALIB_DEFAULT_FLOAT_TYPE,
// 	template<typename, typename> typename map_T = CALIB_DEFAULT_MAP_TYPE>
// using CalibSet_ =
// 	map_T<cv::Size2i, Calibration_<fp_T> >;		// map of resolution <--> calibration
// typedef CalibSet_<>		CalibSet;

// template<
// 	typename fp_T = CALIB_DEFAULT_FLOAT_TYPE,
// 	template<typename, typename> typename cmap_T = CALIB_DEFAULT_MAP_TYPE,
// 	template<typename> typename smap_T = CALIB_DEFAULT_SMAP_TYPE>
// using CalibSMap_ =
// 	smap_T<CalibSet_<fp_T, cmap_T> >;		// stringmap of calibration sets
// typedef CalibSMap_<>	CalibSMap;

// template<
// 	typename fp_T = CALIB_DEFAULT_FLOAT_TYPE,
// 	typename str_T = CALIB_DEFAULT_STRING_TYPE,
// 	template<typename, typename> typename cmap_T = CALIB_DEFAULT_MAP_TYPE,
// 	template<typename, typename> typename nmap_T = CALIB_DEFAULT_MAP_TYPE>
// using CalibMat_ =
// 	nmap_T<str_T, CalibSet_<fp_T, cmap_T> >;	// normal outer map type with string key type
// typedef CalibMat_<>		CalibMat;

template<
	typename fp_T = CALIB_DEFAULT_FLOAT_TYPE,
	typename str_T = CALIB_DEFAULT_STRING_TYPE,
	template<typename, typename> typename map_T = CALIB_DEFAULT_MAP_TYPE>
using CalibMap_ =
CalibMat_<fp_T, str_T, map_T, map_T>;		// ^ but the same map type for inner and outer
typedef CalibMap_<>		CalibMap;

template<typename fp_T = CALIB_DEFAULT_FLOAT_TYPE>
using CalibSList_ = CalibSMap_<fp_T, VecMap, wpi::StringMap>;	// stringmap but with internal map as vecmap
typedef CalibSList_<>	CalibSList;

template<
	typename fp_T = CALIB_DEFAULT_FLOAT_TYPE,
	typename str_T = CALIB_DEFAULT_STRING_TYPE>
using CalibList_ = CalibMap_<fp_T, str_T, VecMap>;		// both maps are vecmap
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
template<typename>
struct CalProps {
	using Float_T = void;
	using String_T = void;

	using Cal_T = void;
	using Set_T = void;
	using Map_T = void;

	static constexpr inline bool
		is_wpi_smap = false,
		is_seq_smap = false,
		is_assoc_smap = false,
		is_seq_cmap = false,
		is_assoc_cmap = false
	;
};
template<
	typename fp_T,
	template<class, class, class...> typename cmap_T>
struct CalProps<CalibSet_<fp_T, cmap_T>> {
	static_assert(std::is_floating_point<fp_T>::value, "Template param error: fp_T must be floating point type.");

	using Float_T = fp_T;
	using String_T = void;

	using Cal_T = Calibration_<fp_T>;
	using Set_T = CalibSet_<fp_T, cmap_T>;
	using Map_T = void;

	static constexpr inline bool
		is_wpi_smap = false,
		is_seq_smap = false,
		is_assoc_smap = false,
		is_seq_cmap = is_same_template<cmap_T, VecMap>::value,
		is_assoc_cmap = any_same_template<cmap_T, std::map, std::unordered_map>::value
	;
};
template<
	typename fp_T,
	template<class, class, class...> typename cmap_T,
	template<class, class...> typename smap_T>
struct CalProps<CalibSMap_<fp_T, cmap_T, smap_T>> {
	static_assert(std::is_floating_point<fp_T>::value, "Template param error: fp_T must be floating point type.");

	using Float_T = fp_T;
	using String_T = CALIB_DEFAULT_STRING_TYPE;

	using Cal_T = Calibration_<fp_T>;
	using Set_T = CalibSet_<fp_T, cmap_T>;
	using Map_T = CalibSMap_<fp_T, cmap_T, smap_T>;

	static constexpr inline bool
		is_wpi_smap = is_same_template<smap_T, wpi::StringMap>::value,
		is_seq_smap = false,
		is_assoc_smap = false,
		is_seq_cmap = is_same_template<cmap_T, VecMap>::value,
		is_assoc_cmap = any_same_template<cmap_T, std::map, std::unordered_map>::value
	;
};
template<
	typename fp_T,
	typename str_T,
	template<class, class, class...> typename cmap_T,
	template<class, class, class...> typename nmap_T>
struct CalProps<CalibMat_<fp_T, str_T, cmap_T, nmap_T>> {
	static_assert(std::is_floating_point<fp_T>::value, "Template param error: fp_T must be floating point type.");

	using Float_T = fp_T;
	using String_T = str_T;

	using Cal_T = Calibration_<fp_T>;
	using Set_T = CalibSet_<fp_T, cmap_T>;
	using Map_T = CalibMat_<fp_T, str_T, cmap_T, nmap_T>;

	static constexpr inline bool
		is_wpi_smap = false,
		is_seq_smap = is_same_template<nmap_T, VecMap>::value,
		is_assoc_smap = any_same_template<nmap_T, std::map, std::multimap, std::unordered_map, std::unordered_multimap>::value,
		is_seq_cmap = is_same_template<cmap_T, VecMap>::value,
		is_assoc_cmap = any_same_template<cmap_T, std::map, std::multimap, std::unordered_map, std::unordered_multimap>::value
	;
};



template<typename Map>
const typename Map::Set_T* findSet(std::string_view name, const Map& map) {
	static_assert(std::is_base_of<__CalibMap, Map>::value, "Template param error - Map must be CalibMap type.");
	if constexpr (CalProps<Map>::is_wpi_smap) {
		return wpi_find_set_impl<Map>(name, map);
	}
	else if constexpr (CalProps<Map>::is_seq_smap) {
		return seq_find_set_impl<Map>(name, map);
	}
	else if constexpr (CalProps<Map>::is_assoc_smap) {
		return assoc_find_set_impl<Map>(name, map);
	}
	return nullptr;
}

template<typename Set>
const typename Set::Cal_T* findCalib(const cv::Size2i& sz, const Set& set) {
	static_assert(std::is_base_of<__CalibSet, Set>::value, "Template param error - Set must be CalibSet type.");
	if constexpr (CalProps<Set>::is_seq_cmap) {
		return seq_find_calib_impl<Set>(sz, set);
	}
	else if constexpr (CalProps<Set>::is_assoc_cmap) {
		return assoc_find_calib_impl<Set>(sz, set);
	}
	return nullptr;
}
template<typename Map>
const typename Map::Cal_T* findCalib(std::string_view name, const cv::Size2i& sz, const Map& map) {
	static_assert(std::is_base_of<__CalibMap, Map>::value, "Template param error - Map must be CalibMap type.");
	if (auto* set = findSet<Map>(name, map)) {
		return findCalib<typename Map::Set_T>(sz, *set);
	}
	return nullptr;
}
template<typename Map>
const typename Map::Cal_T* findAnyCalib(
	std::initializer_list<std::string_view> names, const cv::Size2i& sz, const Map& map
) {
	static_assert(std::is_base_of<__CalibMap, Map>::value, "Template param error - Map must be CalibMap type.");
	for (auto n : names) {
		if (auto* set = findSet<Map>(n, map)) {
			if (auto* cal = findCalib<typename Map::Set_T>(sz, *set)) {
				return cal;
			}
		}
	}
	return nullptr;
}


template<typename Map>
const typename Map::Set_T* wpi_find_set_impl(std::string_view name, const Map& map) {
	static_assert(CalProps<Map>::is_wpi_smap, "Template param error - Map must have wpi::StringMap as smap type.");
	auto it = map.find(name);
	if (it != map.end()) {
		return &it->second;
	}
	return nullptr;
}
template<typename Map>
const typename Map::Set_T* seq_find_set_impl(std::string_view name, const Map& map) {
	static_assert(CalProps<Map>::is_seq_smap, "Template param error - Map must have sequential map type.");
	for(auto& set : map) {
		if(std::string_view(set.first) == name) {
			return &set.second;
		}
	}
	return nullptr;
}
template<typename Map>
const typename Map::Set_T* assoc_find_set_impl(std::string_view name, const Map& map) {
	static_assert(CalProps<Map>::is_assoc_smap, "Template param error - Map must have associative map type.");
	// auto it = map.find(name);
	// if (it != map.end()) {
	// 	return &it->second;
	// }
	return nullptr;
}

template<typename Set>
const typename Set::Cal_T* seq_find_calib_impl(const cv::Size2i& sz, const Set& set) {
	static_assert(CalProps<Set>::is_seq_cmap, "Template param error - Set must have sequential map type.");
	for (auto& calib : set) {
		if (calib.first == sz) {
			return &calib.second;
		}
	}
	return nullptr;
}
template<typename Set>
const typename Set::Cal_T* assoc_find_calib_impl(const cv::Size2i& sz, const Set& set) {
	static_assert(CalProps<Set>::is_assoc_cmap, "Template param error - Set must have associative map type.");
	auto it = set.find(sz);
	if (it != set.end()) {
		return &it->second;
	}
	return nullptr;
}