#pragma once

#include <vector>

#include <opencv2/opencv.hpp>

#include "../core/vs-1.x-archive/weightedsubtraction.h"
#include "../core/vs-1.x-archive/visionserver.h"
#include "../core/vs-1.x-archive/processing.h"
#include "../core/vs-1.x-archive/target.h"
//#include "vision.h"

#include "../defines.h"


class Test6x6 : public Target<4> {
public:
	Test6x6() : Target<4>({		// world points in clockwise order
		cv::Point3f(0.25f, 0.25f, 0.f), 	//top-right
		cv::Point3f(0.25f, -0.25f, 0.f),	//bottom-right
		cv::Point3f(-0.25f, -0.25f, 0.f),	//bottom-left
		cv::Point3f(-0.25f, 0.25f, 0.f),	//top-left
	}, "Test Retroreflective Square") {}

};



class BBoxDemo : public VPipeline, public WeightedSubtraction<VThreshold::LED::BLUE>, public Contours {
public:
	BBoxDemo(VisionServer& server);
	BBoxDemo(const BBoxDemo& other) = delete;

	void process(cv::Mat& io_frame) override;

private:
	cv::Rect boundingbox;

};
class SquareTargetPNP : public VPipeline, public WeightedSubtraction<VThreshold::LED::BLUE>, public Contours {
public:
	SquareTargetPNP(VisionServer& server);
	SquareTargetPNP(const SquareTargetPNP& other) = delete;

	void process(cv::Mat& io_frame) override;

private:
	std::vector<cv::Point> target_points;
	Test6x6 reference_points;

	cv::Mat_<float> rvec = cv::Mat_<float>(1, 3), tvec = rvec/*, rmat = cv::Mat_<float>(3, 3)*/;

	std::vector<cv::Point2d> projection2d;
	const std::vector<cv::Point3d> projection3d{
		cv::Point3f(0.25f, 0.25f, 0.25f), 
		cv::Point3f(0.25f, -0.25f, 0.25f),
		cv::Point3f(-0.25f, -0.25f, 0.25f),
		cv::Point3f(-0.25f, 0.25f, 0.25f), 
	};

};