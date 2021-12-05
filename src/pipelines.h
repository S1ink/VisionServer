#pragma once

#include <networktables/NetworkTableInstance.h>
#include <networktables/EntryListenerFlags.h>
#include <networktables/NetworkTableEntry.h>
#include <networktables/NetworkTable.h>
#include <wpi/StringRef.h>
#include <wpi/json.h>
#include <wpi/raw_ostream.h>

#include "cameraserver/CameraServer.h"

#include <opencv2/opencv.hpp>

#include <iostream>
#include <chrono>

#include "extras/resources.h"
#include "extras/stats.h"

#include "visionserver.h"
#include "mem.h"

// DONT COPY INSTANCES THEY ARE EXTREMELY HEAVY
class TestPipeline : public PipelineBase {
public:
	TestPipeline(const VisionServer* server);
	TestPipeline(const TestPipeline& other) = delete;	// only remove this if absolutely necessary

	void resizeBuffers(cv::Size size);
	inline bool cmpSize(const cv::Size& newsize) {
		return this->resolution == newsize;
	}

	void process(cv::Mat& io_frame, bool show_binary = false) override;

private: 
	const std::shared_ptr<nt::NetworkTable> table{nt::NetworkTableInstance::GetDefault().GetTable("Test Pipeline")};
	//std::array<bool, 1> options{false};
	double weight{0.5};
	uint8_t threshold{100};
	size_t scale{4};

	cv::Size resolution;
	cv::Mat buffer, binary;
	std::array<cv::Mat, 3> channels;

	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Point> poly_contour;
	cv::Rect boundingbox;
	double largest{0.f}, area{0.f};
	int16_t target{0};

};