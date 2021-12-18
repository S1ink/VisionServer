#pragma once

#include <networktables/NetworkTable.h>
#include <wpi/raw_ostream.h>

#include <opencv2/opencv.hpp>

#include <vector>
#include <array>

#include "extras/resources.h"

#include "visionserver.h"
#include "processing.h"

// template<size_t corners>
// struct Target {
// 	Target(const std::array<cv::Point3f, corners>& world_pts);

// 	std::array<cv::Point2f, corners> points;		// actual points 
// 	const std::array<cv::Point3f, corners> world;	// world points that relate to the above 

// 	size_t getSize();

// 	virtual void sort(const std::vector<cv::Point2i>& contour);
// 	void rescale(double scale);	// multiplies points (x and y) by scale for all points
// 	std::array<cv::Point2f, corners> getRescaled(double scale);	// returns rescaled array of points, does not alter internal array

// };

// #include "processing.inc"

// class DummyBase {
// public:
// 	DummyBase(VisionServer& server);
// 	//virtual ~DummyBase() = default;

// 	virtual void dummyfunc() = 0;

// };

// class Dummy : public DummyBase {
// public:
// 	Dummy(VisionServer& server);
// 	Dummy(const Dummy& other) = delete;
// 	//virtual ~Dummy() = default;

// 	void dummyfunc() override;

// protected:
// 	double weight{0.5};
// 	uint8_t thresh{50};
// 	size_t scale{4};

// 	cv::Mat buffer, binary;
// 	std::array<cv::Mat, 3> channels;

// };

class TestPipeline : public VPipeline, public WSThreshold<VThreshold::LED::BLUE> {
public:
	TestPipeline(VisionServer& server);
	TestPipeline(const TestPipeline& other) = delete;

	void process(cv::Mat& io_frame, bool debug = false) override;

private:
	bool cvh{false}, apdp{false};

	double largest{0.f}, area{0.f};
	int16_t target{0};

	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Point> target_points;

	class TestPoints : public Target<4> {
	public:
		TestPoints() : Target<4>({
			cv::Point3f(-0.25f, 0.25f, 0.f),	//top-left
			cv::Point3f(0.25f, 0.25f, 0.f), 	//top-right
			cv::Point3f(-0.25f, -0.25f, 0.f), 	//bottom-left
			cv::Point3f(0.25f, -0.25f, 0.f)		//bottom-right
		}) {}

		//void sort(const std::vector<cv::Point2i>& contour) override;
	private:
		cv::Point2f center, a, b;
		size_t limit;
	} reference_points;
};

class BBoxDemo : public VPipeline, public WSThreshold<VThreshold::LED::BLUE>, public ContourPipe {
public:
	//using VPipeline::VPipeline;
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
	class Square : public Target<4> {
	public:
		Square() : Target<4>({				// world points in clockwise order
			cv::Point3f(0.25f, 0.25f, 0.f), 	//top-right
			cv::Point3f(0.25f, -0.25f, 0.f),	//bottom-right
			cv::Point3f(-0.25f, -0.25f, 0.f),	//bottom-left
			cv::Point3f(-0.25f, 0.25f, 0.f),	//top-left
		}) {}

		//void sort(const std::vector<cv::Point2i>& contour) override;	// sorts points to be clockwise (match the world points)
	private:
		cv::Point2f center, a, b;
		size_t limit;
	} reference_points;

	cv::Mat_<float> rvec = cv::Mat_<float>(1, 3), tvec = rvec/*, rmat = cv::Mat_<float>(3, 3)*/;

	std::vector<cv::Point2d> projection2d;
	const std::vector<cv::Point3d> projection3d{
		cv::Point3f(0.25f, 0.25f, 0.25f), 
		cv::Point3f(0.25f, -0.25f, 0.25f),
		cv::Point3f(-0.25f, -0.25f, 0.25f),
		cv::Point3f(-0.25f, 0.25f, 0.25f), 
	};

};



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