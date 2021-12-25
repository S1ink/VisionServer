#pragma once

#include <opencv2/opencv.hpp>

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

	virtual const cv::Mat& threshold(const cv::Mat& i_frame) = 0;
	virtual const cv::Mat& getBinary() = 0;

	enum class LED {
		BLUE = 0, 
		GREEN = 1, 
		RED = 2
	};
};



template<VThreshold::LED color>		// 'Weighted Subtraction Threshold' pipeline
class WeightedSubtraction : public VThreshold {
public:
	//WSThreshold() = default;
	WeightedSubtraction(cv::Size frame_sz, std::shared_ptr<nt::NetworkTable> table);
	WeightedSubtraction(const WeightedSubtraction& other) = delete;
	// WSThreshold(WSThreshold&&);
	// void operator=(const WSThreshold&) = delete;
	// void operator=(WSThreshold&&);

	const cv::Mat& threshold(const cv::Mat& i_frame) override;
	const cv::Mat& getBinary() override;

	size_t getScale() const;
	uint8_t getThresh() const;
	void setThresh(uint8_t t);
	double getWeight() const;
	void setWeight(double w);

protected:
	void resizeBuffers(cv::Size size);

	static const std::array<std::array<uint8_t, 2>, 3> weighted_array;

	cv::Mat buffer, binary;
	std::array<cv::Mat, 3> channels;

	double weight{0.5};
	uint8_t thresh{50};
	size_t scale{4};

};
class Contours {		// Provides contour functions and contour storage
public:
	Contours() = default;	// maybe take in a networktable to publish settings for area threshold?
	Contours(const Contours& other) = delete;

	size_t findContours(const cv::Mat& binary_frame, int mode = cv::RETR_EXTERNAL, int method = cv::CHAIN_APPROX_SIMPLE);	// returns how many were found
	size_t findLargest(const cv::Mat& binary_frame, int mode = cv::RETR_EXTERNAL, int method = cv::CHAIN_APPROX_SIMPLE);	// returns index of the largest
	size_t findLargestThreshold(const cv::Mat& binary_frame, double area, int mode = cv::RETR_EXTERNAL, int method = cv::CHAIN_APPROX_SIMPLE);	// find largest that has a greater area than 'area', returns index

	size_t getTargetIdx() const;
	const std::vector<cv::Point2i>& getTarget() const;
	inline bool validTarget() {
		return this->target_idx >= 0;
	}

	// function for finding corner points and/or reducing points?

protected:
	std::vector<std::vector<cv::Point2i> > contours;

	double area_largest{0.f}, area_buff{0.f};
	int16_t target_idx{-1};		// DONT CHANGE THIS TO SIZE_T YOU IDIOT!!!

};

#include "processing.inc"