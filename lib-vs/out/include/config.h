#pragma once

#include <type_traits>
#include <vector>
#include <array>
#include <unordered_map>
#include <string>

#include <opencv2/core/types.hpp>

#include <wpi/json.h>

#include "visioncamera.h"


#define FRC_CONFIG	"/boot/frc.json"

template<
	typename fp_T = float,
	typename str_T = std::string,
	template<class...> typename map_T = std::unordered_map>
using CalibMap_ =
	std::vector<
		map_T<
			str_T,	std::vector<
						map_T<
							cv::Size2i, std::array<cv::Mat_<fp_T>, 2>
						>
					>
		>
	>
; typedef CalibMap_<>	CalibMap;

template<
	typename fp_T = float,
	typename str_T = const char*>
using CalibList_	=	CalibMap_<fp_T, str_T, std::pair>
; typedef CalibList_<>	CalibList;


/**
 * Reads a config json and creates appropriate VisionCameras in the supplied vector
 * @param cameras The output vector in which cameras will be created
 * @param file The path to the json, default is "/boot/frc.json"
 * @return false if there was an error
*/
bool readConfig(std::vector<VisionCamera>& cameras, const char* file = FRC_CONFIG);