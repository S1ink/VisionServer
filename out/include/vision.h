#pragma once

#include <networktables/NetworkTableInstance.h>
#include <networktables/NetworkTable.h>
#include <cameraserver/CameraServer.h>
#include <wpi/Twine.h>

#include <opencv2/core/types.hpp>

#include <type_traits>

#include "visioncamera.h"
#include "tools/src/resources.h"

// helper functions that extend cameraserver and opencv functionality

CE_STR _default = "/boot/frc.json";
bool readConfig(std::vector<VisionCamera>& cameras, const char* file = _default);

cs::CvSink switchedCameraVision(
	const std::vector<VisionCamera>& cameras, 
	std::shared_ptr<nt::NetworkTable> table = nt::NetworkTableInstance::GetDefault().GetTable("Vision")
);

static inline cv::Size getResolution(cs::VideoMode vm) {
    return cv::Size(vm.height, vm.width);
}

template<typename num_t>
void addNetTableVar(num_t& var, const wpi::Twine& name, std::shared_ptr<nt::NetworkTable> table);
void addNetTableVar(bool& var, const wpi::Twine& name, std::shared_ptr<nt::NetworkTable> table);

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