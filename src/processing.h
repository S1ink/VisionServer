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

	virtual void threshold(const cv::Mat& i_frame) = 0;

	enum class LED {
		BLUE, GREEN, RED
		//, TOTAL, ERROR
	};
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

protected:
	void resizeBuffers(cv::Size size);

	static const std::array<std::array<uint8_t, 2>, 3> weighted_array;

	cv::Mat buffer, binary;
	std::array<cv::Mat, 3> channels;

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
	size_t findLargest_A(const cv::Mat& binary_frame, double area);	// find largest that has a greater area than 'area', returns index

	size_t getTargetIdx() const;
	const std::vector<cv::Point2i>& getTarget() const;
	inline bool validTarget() {
		return this->target_idx >= 0;
	}

	// function for finding corner points and/or reducing points?

protected:
	std::vector<std::vector<cv::Point2i> > contours;

	double area_largest{0.f}, area_buff{0.f};
	int16_t target_idx{-1};					// DONT CHANGE THIS TO SIZE_T!!!!!!!!!

};

#include "processing.inc"