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
#include "vision.h"
#include "mem.h"

class WSTPipeline : public PipelineBase {	// 'Weighted Subtraction Threshold' pipeline base
protected:
	WSTPipeline(VisionServer* server, std::shared_ptr<nt::NetworkTable> table);
	WSTPipeline(const WSTPipeline& other) = delete;

	virtual void resizeBuffers(cv::Size size);
	void threshold(cv::Mat& io_frame);

	enum class PColor {	//template param?
		BLUE, GREEN, RED, 
		TOTAL, ERROR
	} color{PColor::BLUE};
	static const std::array<std::array<uint8_t, 2>, 3> weighted_array;

	double weight{0.5};
	uint8_t thresh{50};
	size_t scale{4};	// template param?

	cv::Mat buffer, binary;
	std::array<cv::Mat, 3> channels;

};

class BBoxDemo : public WSTPipeline {
public:
	BBoxDemo(VisionServer* server);
	BBoxDemo(const BBoxDemo& other) = delete;

	void process(cv::Mat& io_frame, bool output_binary = false) override;

private:
	const std::shared_ptr<nt::NetworkTable> table{nt::NetworkTableInstance::GetDefault().GetTable("BoundingBox Demo Pipeline")};

	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Point> t_contour;
	cv::Rect boundingbox;
	double largest{0.f}, area{0.f};
	size_t target{0};

};

// class SquareTargetPNP : public WSTPipeline {
// public:
// 	SquareTargetPNP(VisionServer* server);
// 	SquareTargetPNP(const SquareTargetPNP& other) = delete;

// 	void resizeBuffers(cv::Size size);

// 	void process(cv::Mat& io_frame, bool output_binary = false) override;

// private:

// };

// DONT COPY INSTANCES THEY ARE EXTREMELY HEAVY
class TestPipeline : public PipelineBase {
public:
	TestPipeline(VisionServer* server);
	TestPipeline(const TestPipeline& other) = delete;	// only remove this if absolutely necessary

	void resizeBuffers(cv::Size size);
	inline bool cmpSize(const cv::Size& newsize) {
		return this->resolution == newsize;
	}

	void process(cv::Mat& io_frame, bool output_binary = false) override;

private: 
	class SquareTarget {
	public:
		union square {
			std::array<cv::Point2f, 4> array;
			square() {}
			struct data {
				cv::Point2f top_left, top_right, bottom_left, bottom_right;
			} points;
		} data;

		void sortPoints(const std::vector<cv::Point>& points);
		cv::Point2d findCenter();

		void scaleUp(size_t scale);
		void scaleDown(size_t scale);
	};

	const std::shared_ptr<nt::NetworkTable> table{nt::NetworkTableInstance::GetDefault().GetTable("Test Pipeline")};
	//std::array<bool, 1> options{false};
	double weight{0.5};
	uint8_t threshold{100};
	size_t scale{4};

	cv::Size resolution;
	cv::Mat buffer, binary;
	std::array<cv::Mat, 3> channels;

	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Point> t_contour;
	cv::Rect boundingbox;
	double largest{0.f}, area{0.f};
	int16_t target{0};

	//solvePnP test data

	SquareTarget image_corners;
	const std::array<cv::Point3f, 4> world_corners = {
		cv::Point3f(-0.25f, 0.25f, 0.f),	//top-left
		cv::Point3f(0.25f, 0.25f, 0.f), 	//top-right
		cv::Point3f(-0.25f, -0.25f, 0.f), 	//bottom-left
		cv::Point3f(0.25f, -0.25f, 0.f)		//bottom-right
	};
	const cv::Mat_<float> camera_matrix = (
		cv::Mat_<float>(3,3) << 
		548.3660465470496, 0.0, 318.36628869739957, 
		0.0, 546.1695458061688, 238.9800848148716, 
		0.0, 0.0, 1.0
	);
	const cv::Mat_<float> distortion = (
		cv::Mat_<float>(1, 5) << 
		0.02457462254785843, -0.24628433420855583, 0.000186658140280956, -5.80865061225857e-05, 0.4151871513521476
	);
	cv::Mat_<float> rvec = cv::Mat_<float>(1, 3), tvec = rvec, rmat = cv::Mat_<float>(3, 3);

	const std::vector<cv::Point3d> perpendicular3d = {
		cv::Point3f(-0.25f, 0.25f, 0.25f),
		cv::Point3f(0.25f, 0.25f, 0.25f), 
		cv::Point3f(-0.25f, -0.25f, 0.25f), 
		cv::Point3f(0.25f, -0.25f, 0.25f)
	};
	std::vector<cv::Point2d> perpendicular2d;

};