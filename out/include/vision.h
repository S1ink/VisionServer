#pragma once

#include <networktables/NetworkTableInstance.h>
#include <networktables/NetworkTable.h>
#include <cameraserver/CameraServer.h>
#include <wpi/Twine.h>

#include <opencv2/core/types.hpp>

#include <type_traits>

#include "tools/src/resources.h"
#include "visioncamera.h"
#include "defines.h"

// helper functions that extend cameraserver and opencv functionality

CE_STR _default = "/boot/frc.json";
/**
 * Reads a config json and creates appropriate VisionCameras in the supplied vector
 * @param cameras The output vector in which cameras will be created
 * @param file The path to the json, default is "/boot/frc.json"
 * @return false if there was an error
*/
bool readConfig(std::vector<VisionCamera>& cameras, const char* file = _default);

/**
 * Starts a stream and creates a networktable value for changing which camera is streamed
 * @param cameras A vector of VisionCameras to be used
 * @param table A networktable in which to put the camera index entries
 * @return A cs::CvSink which is is the source for the stream
*/
cs::CvSink switchedCameraVision(
	const std::vector<VisionCamera>& cameras, 
	std::shared_ptr<nt::NetworkTable> table = nt::NetworkTableInstance::GetDefault().GetTable("Vision")
);

/**
 * Extracts the height and width of a cs::VideoMode object and converts to cv::Size
 * @param vm The cs::VideoMode object to get resolution from
 * @return The resolutuion in cv::Size format
*/
static inline cv::Size getResolution(cs::VideoMode vm) { return cv::Size(vm.height, vm.width); }

/**
 * Adds a networktable entry to the provided networktable with a listener that will update the variable to match the networktable entry
 * @param num_t Any numeric type - does not need to be explicitly provided
 * @param var A reference to the numeric variable
 * @param name The name of the entry
 * @param table The root networktable
*/
template<typename num_t>
void addNetTableVar(num_t& var, const wpi::Twine& name, std::shared_ptr<nt::NetworkTable> table);
/**
 * Adds a networktable entry to the provided networktable with a listener that will update the variable to match the networktable entry
 * @param var A reference to the boolean variable
 * @param name The name of the entry
 * @param table The root networktable
*/
void addNetTableVar(bool& var, const wpi::Twine& name, std::shared_ptr<nt::NetworkTable> table);

/**
 * Divides a both parts of a cv::Size object by the specified scale
 * @param num_t The numberic type used in the cv::Size input (and function output) - does not need to be explicitly provided
 * @param input The input size
 * @param scale The amount to be divided by
 * @return A new cv::Size representing the division
*/
template<typename num_t> static inline
cv::Size_<num_t> operator/(cv::Size_<num_t> input, size_t scale) {
	static_assert(std::is_arithmetic<num_t>::value, "Template parameter (num_t) must be arithemetic type");
	return cv::Size_<num_t>(input.width/scale, input.height/scale);
}

template<typename num_t>
void rescale(std::vector<cv::Point_<num_t> >& points, size_t scale);	// scales up (multiplies by scale)
template<typename num_t>
void _rescale(std::vector<cv::Point_<num_t> >& points, size_t scale);	// scales down with a size value (multiplies by 1.0/value)
template<typename num_t>
void rescale(std::vector<cv::Point_<num_t> >& points, double scale);	// scales up or down (multiplies by scale)

template<typename num_t>
cv::Point_<num_t> findCenter(const std::vector<cv::Point_<num_t> >& contour);
template<typename onum_t, typename inum_t>
cv::Point_<onum_t> findCenter(const std::vector<cv::Point_<inum_t> >& contour);
template<typename num_t>
cv::Point3_<num_t> findCenter3D(const std::vector<cv::Point3_<num_t> >& contour);
template<typename onum_t, typename inum_t>
cv::Point3_<onum_t> findCenter3D(const std::vector<cv::Point3_<inum_t> >& contour);

template<typename num_t>
void reorderClockWise(std::vector<cv::Point_<num_t> >& points);
template<typename num_t>
void reorderCClockWise(std::vector<cv::Point_<num_t> >& points);

template<typename num_t, size_t s>
std::array<cv::Point3_<num_t>, s> operator+(const std::array<cv::Point_<num_t>, s>& base, num_t depth);
template<typename num_t, size_t s>
std::array<cv::Point3_<num_t>, s> operator+(const std::array<cv::Point3_<num_t>, s>& base, num_t depth);
template<typename num_t, size_t s>
void operator+=(std::array<cv::Point3_<num_t>, s>& base, num_t depth);	// extend by depth amount
template<typename num_t>
std::vector<cv::Point3_<num_t> > operator+(const std::vector<cv::Point_<num_t> >& base, num_t depth);
template<typename num_t>
std::vector<cv::Point3_<num_t> > operator+(const std::vector<cv::Point3_<num_t> >& base, num_t depth);
template<typename num_t>
void operator+=(std::vector<cv::Point3_<num_t>>& base, num_t depth); 
template<typename num_t, size_t s>
const auto extend_array = operator+=<num_t, s>;	// an alias
template<typename num_t>
const auto extend_vector = operator+=<num_t>;

#include "vision.inc"