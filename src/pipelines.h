#pragma once

#include <networktables/NetworkTable.h>

#include <opencv2/opencv.hpp>

#include <vector>
#include <array>
#include <type_traits>

#include "extras/resources.h"

#include "visionserver.h"
#include "processing.h"
#include "targets.h"

class BBoxDemo : public VPipeline, public WSThreshold<VThreshold::LED::BLUE>, public ContourPipe {
public:
	BBoxDemo(VisionServer& server);
	BBoxDemo(const BBoxDemo& other) = delete;

	void process(cv::Mat& io_frame, bool debug = false) override;

private:
	cv::Rect boundingbox;

};
class SquareTargetPNP : public VPipeline, public WSThreshold<VThreshold::LED::BLUE>, public ContourPipe {
public:
	SquareTargetPNP(VisionServer& server);
	SquareTargetPNP(const SquareTargetPNP& other) = delete;

	void process(cv::Mat& io_frame, bool debug = false) override;

private:
	std::vector<cv::Point> target_points;
	TestingSquare reference_points;

	cv::Mat_<float> rvec = cv::Mat_<float>(1, 3), tvec = rvec/*, rmat = cv::Mat_<float>(3, 3)*/;

	std::vector<cv::Point2d> projection2d;
	const std::vector<cv::Point3d> projection3d{
		cv::Point3f(0.25f, 0.25f, 0.25f), 
		cv::Point3f(0.25f, -0.25f, 0.25f),
		cv::Point3f(-0.25f, -0.25f, 0.25f),
		cv::Point3f(-0.25f, 0.25f, 0.25f), 
	};

};

// template <template <typename...> class BaseTemplate, typename Derived>
// struct test_base_template<BaseTemplate, Derived, std::enable_if_t<std::is_class_v<Derived> > > : Derived
// {
//     template<typename...T>
//     static constexpr std::true_type test(BaseTemplate<T...> *);
//     static constexpr std::false_type test(...);
//     using is_base = decltype(test((Derived *) nullptr));
// };

template<typename target_t, VThreshold::LED color = VThreshold::LED::BLUE>
class TargetSolver : public VPipeline, public WSThreshold<color>, public ContourPipe {
	//static_assert(std::is_convertible<Target<0>, target_t>::value, "Target type (target_t) must inherit from Target<t,c>");
public:
	TargetSolver(VisionServer& server);
	TargetSolver(const TargetSolver& other) = delete;

	void process(cv::Mat& io_frame, bool debug = false) override;

private:
	std::vector<cv::Point> target_points;
	target_t reference_points;

	cv::Mat_<float> rvec = cv::Mat_<float>(1, 3), tvec = rvec/*, rmat = cv::Mat_<float>(3, 3)*/;

};

#include "pipelines.inc"



// DONT COPY INSTANCES THEY ARE EXTREMELY HEAVY
// class TestPipeline : public PipelineBase {
// public:
// 	TestPipeline(VisionServer* server);
// 	TestPipeline(const TestPipeline& other) = delete;	// only remove this if absolutely necessary

// 	void resizeBuffers(cv::Size size);
// 	inline bool cmpSize(const cv::Size& newsize) {
// 		return this->resolution == newsize;
// 	}

// 	void process(cv::Mat& io_frame, bool output_binary = false) override;

// private: 
// 	class SquareTarget {
// 	public:
// 		union square {
// 			std::array<cv::Point2f, 4> array;
// 			square() {}
// 			struct data {
// 				cv::Point2f top_left, top_right, bottom_left, bottom_right;
// 			} points;
// 		} data;

// 		void sortPoints(const std::vector<cv::Point>& points);
// 		cv::Point2d findCenter();

// 		void scaleUp(size_t scale);
// 		void scaleDown(size_t scale);
// 	};

// 	const std::shared_ptr<nt::NetworkTable> table{nt::NetworkTableInstance::GetDefault().GetTable("Test Pipeline")};
// 	//std::array<bool, 1> options{false};
// 	double weight{0.5};
// 	uint8_t threshold{100};
// 	size_t scale{4};

// 	cv::Size resolution;
// 	cv::Mat buffer, binary;
// 	std::array<cv::Mat, 3> channels;

// 	std::vector<std::vector<cv::Point> > contours;
// 	std::vector<cv::Point> t_contour;
// 	cv::Rect boundingbox;
// 	double largest{0.f}, area{0.f};
// 	int16_t target{0};

// 	//solvePnP test data

// 	SquareTarget image_corners;
// 	const std::array<cv::Point3f, 4> world_corners = {
// 		cv::Point3f(-0.25f, 0.25f, 0.f),	//top-left
// 		cv::Point3f(0.25f, 0.25f, 0.f), 	//top-right
// 		cv::Point3f(-0.25f, -0.25f, 0.f), 	//bottom-left
// 		cv::Point3f(0.25f, -0.25f, 0.f)		//bottom-right
// 	};
// 	const cv::Mat_<float> camera_matrix = (
// 		cv::Mat_<float>(3,3) << 
// 		548.3660465470496, 0.0, 318.36628869739957, 
// 		0.0, 546.1695458061688, 238.9800848148716, 
// 		0.0, 0.0, 1.0
// 	);
// 	const cv::Mat_<float> distortion = (
// 		cv::Mat_<float>(1, 5) << 
// 		0.02457462254785843, -0.24628433420855583, 0.000186658140280956, -5.80865061225857e-05, 0.4151871513521476
// 	);
// 	cv::Mat_<float> rvec = cv::Mat_<float>(1, 3), tvec = rvec, rmat = cv::Mat_<float>(3, 3);

// 	const std::array<cv::Point3d, 4> perpendicular3d = {
// 		cv::Point3f(-0.25f, 0.25f, 0.25f),
// 		cv::Point3f(0.25f, 0.25f, 0.25f), 
// 		cv::Point3f(-0.25f, -0.25f, 0.25f), 
// 		cv::Point3f(0.25f, -0.25f, 0.25f)
// 	};
// 	std::vector<cv::Point2d> perpendicular2d;

// };