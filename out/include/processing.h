#pragma once

#include <networktables/NetworkTable.h>

#include <opencv2/opencv.hpp>

#include <vector>

#include "visionserver.h"

class VThreshold {	// Vision Thresholding base interface
public:
	VThreshold(VisionServer&, std::shared_ptr<nt::NetworkTable>) {}
	VThreshold(VisionServer&) {}
	virtual ~VThreshold() = default;

	virtual const cv::Mat& threshold(const cv::Mat& i_frame) = 0;
	virtual void fromBinary(cv::Mat& o_frame) const = 0;
	virtual size_t getScale() const = 0;

	inline virtual const cv::Mat& getBinary() const {
		return this->binary;
	}

	enum class LED {
		BLUE = 0, 
		GREEN = 1, 
		RED = 2
	};

protected:
	cv::Mat binary;

};

template<typename num_t>
inline num_t operator~(VThreshold::LED color) { return static_cast<num_t>(color); }
inline uint8_t operator~(VThreshold::LED color) { return static_cast<uint8_t>(color); }

class Contours {		// Provides contour functions and contour storage
public:
	Contours() = default;	// maybe take in a networktable to publish settings for area threshold?
	Contours(const Contours& other) = delete;

	size_t findContours(const cv::Mat& binary_frame, int mode = cv::RETR_EXTERNAL, int method = cv::CHAIN_APPROX_SIMPLE);	// returns how many were found
	size_t findLargest(const cv::Mat& binary_frame, int mode = cv::RETR_EXTERNAL, int method = cv::CHAIN_APPROX_SIMPLE);	// returns index of the largest
	size_t findLargestThreshold(const cv::Mat& binary_frame, double area, int mode = cv::RETR_EXTERNAL, int method = cv::CHAIN_APPROX_SIMPLE);	// find largest that has a greater area than 'area', returns index

	inline size_t getTargetIdx() const { return this->target_idx; }
	inline const std::vector<cv::Point2i>& getTarget() const { return this->contours[this->target_idx]; }
	inline bool validTarget() { return this->target_idx >= 0; }

	// function for finding corner points and/or reducing points?

protected:
	std::vector<std::vector<cv::Point2i> > contours;

	double area_largest{0.f}, area_buff{0.f};
	int16_t target_idx{-1};		// DONT CHANGE THIS TO SIZE_T YOU IDIOT!!!

};