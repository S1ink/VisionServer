#pragma once

#include <algorithm>
#include <math.h>
#include <vector>
#include <array>

#include "opencv2/opencv.hpp"

#include "extras/resources.h"
#include "visionserver.h"
#include "vision.h"
#include "mem.h"

class WSTPipeline : public PipelineBase {	// 'Weighted Subtraction Threshold' pipeline base
protected:
	WSTPipeline(VisionServer* server, std::shared_ptr<nt::NetworkTable> table);
	WSTPipeline(const WSTPipeline& other) = delete;

	virtual void resizeBuffers(cv::Size size);
	void threshold(const cv::Mat& io_frame);
	void process(cv::Mat& io_frame, bool output_binary = false) override;

	enum class PColor {	//template param?
		BLUE, GREEN, RED
		//, TOTAL, ERROR
	} color{PColor::BLUE};
	static const std::array<std::array<uint8_t, 2>, 3> weighted_array;

	double weight{0.5};
	uint8_t thresh{50};
	size_t scale{4};	// template param?

	cv::Mat buffer, binary;
	std::array<cv::Mat, 3> channels;

};
class ContourPipeline {		// Provides contour functions and contour storage
protected:
	ContourPipeline();
	ContourPipeline(const ContourPipeline& other) = delete;

	size_t findContours(const cv::Mat& binary_frame);	// returns how many were found
	size_t findLargest(const cv::Mat& binary_frame);	// returns index of the largest
	size_t findLargestGT(const cv::Mat& binary_frame, double area);	// find largest that has a greater area than 'area', returns index

	std::vector<std::vector<cv::Point2i> > contours;
	double largest{0.f}, area{0.f};
	size_t target{0};

};

template<size_t corners>
struct Target {
	Target(const std::array<cv::Point3f, corners>& world_pts);

	std::array<cv::Point2f, corners> points;
	const std::array<cv::Point3f, corners> world;

	size_t getSize();

	virtual void sort(const std::vector<cv::Point2i>& contour);
	void rescale(double scale);	// multiplies points (x and y) by scale for all points
	std::array<cv::Point2f, corners> getRescaled(double scale);	// returns rescaled array of points, does not alter internal array
};

#include "processing.inc"