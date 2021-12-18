#pragma once

#include <opencv2/opencv.hpp>

#include <algorithm>
#include <math.h>
#include <vector>
#include <array>

#include "extras/resources.h"
#include "visionserver.h"
#include "vision.h"
#include "mem.h"

class VThreshold {		// Vision Thresholding base interface (pointless now but may be used in the future)
public:
	VThreshold() = default;
	virtual ~VThreshold() = default;

	virtual void threshold(const cv::Mat& i_frame) = 0;

	enum class LED {
		BLUE, GREEN, RED
		//, TOTAL, ERROR
	};

};
template<size_t corners>
struct Target {
	Target(const std::array<cv::Point3f, corners>& world_pts);

	std::array<cv::Point2f, corners> points;		// actual points 
	const std::array<cv::Point3f, corners> world;	// world points that relate to the above 

	size_t getSize();

	virtual void sort(const std::vector<cv::Point2i>& contour);
	void rescale(double scale);	// multiplies points (x and y) by scale for all points
	std::array<cv::Point2f, corners> getRescaled(double scale);	// returns rescaled array of points, does not alter internal array

};



template<VThreshold::LED color>			// 'Weighted Subtraction Threshold' pipeline
struct WSThreshold : public VThreshold {
public:
	//WSThreshold() = default;
	WSThreshold(cv::Size frame_sz, std::shared_ptr<nt::NetworkTable> table);
	WSThreshold(const WSThreshold& other) = delete;

	void threshold(const cv::Mat& i_frame) override;

	size_t getScale() const;
	uint8_t getThresh() const;
	void setThresh(uint8_t t);
	double getWeight() const;
	void setWeight(double w);

	cv::Mat buffer, binary;
	std::array<cv::Mat, 3> channels;

protected:
	void resizeBuffers(cv::Size size);

	static const std::array<std::array<uint8_t, 2>, 3> weighted_array;

	double weight{0.5};
	uint8_t thresh{50};
	size_t scale{4};

};
struct ContourPipe {		// Provides contour functions and contour storage
public:
	ContourPipe() = default;	// maybe take in a networktable to publish settings for area threshold?
	ContourPipe(const ContourPipe& other) = delete;

	size_t findContours(const cv::Mat& binary_frame);	// returns how many were found
	size_t findLargest(const cv::Mat& binary_frame);	// returns index of the largest
	size_t findLargestGT(const cv::Mat& binary_frame, double area);	// find largest that has a greater area than 'area', returns index

	size_t getTarget() const;

	// function for finding corner points and/or reducing points?

	std::vector<std::vector<cv::Point2i> > contours;

protected:
	double largest{0.f}, area{0.f};
	size_t target{0};

};

#include "processing.inc"