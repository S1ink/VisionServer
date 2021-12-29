#pragma once

#include <networktables/NetworkTable.h>
#include <opencv2/opencv.hpp>

#include <array>

#include "visionserver.h"
#include "processing.h"

template<VThreshold::LED color>	// 'Weighted Subtraction Threshold' thresholding subpipeline
class WeightedSubtraction : public VThreshold {
public:
	//WSThreshold() = default;
	WeightedSubtraction(VisionServer& server, std::shared_ptr<nt::NetworkTable> table);
	WeightedSubtraction(const WeightedSubtraction& other) = delete;
	// WSThreshold(WSThreshold&&);
	// void operator=(const WSThreshold&) = delete;
	// void operator=(WSThreshold&&);

	const cv::Mat& threshold(const cv::Mat& i_frame) override;
	void fromBinary(cv::Mat& o_frame) const override;
	size_t getScale() const override;

	uint8_t getThresh() const;
	void setThresh(uint8_t t);
	double getWeight() const;
	void setWeight(double w);

protected:
	void resizeBuffers(cv::Size size);

	static const std::array<std::array<uint8_t, 2>, 3> weighted_array;

	cv::Mat buffer;
	std::array<cv::Mat, 3> channels;

	double weight{0.5};
	uint8_t thresh{50};
	size_t scale{4};

};

#include "weightedsubtraction.inc"