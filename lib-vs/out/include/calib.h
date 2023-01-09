#pragma once

#include <vector>
#include <array>
#include <string>
#include <unordered_map>

#include <opencv2/core/types.hpp>


template<typename... Ts>
using VecMap = std::vector<std::pair<Ts...> >;

template<
	typename fp_T = float,
	typename str_T = std::string,
	template<class...> typename map_T = std::unordered_map>
using CalibMap_ =
	map_T<
		str_T,	map_T<
					cv::Size2i, std::array<cv::Mat_<fp_T>, 2>
				>
	>
; typedef CalibMap_<>	CalibMap;

template<
	typename fp_T = float,
	typename str_T = const char*>
using CalibList_	=	CalibMap_<fp_T, str_T, VecMap>;
typedef CalibList_<>	CalibList;