#pragma once

#include <vector>

#include <opencv2/opencv.hpp>

#include "../weightedsubtraction.h"
#include "../visionserver.h"
#include "../processing.h"
#include "../target.h"


class Test6x6 : public vs1::Target<4> {
public:
	Test6x6() : Target<4>({		// world points in clockwise order
		cv::Point3f(0.25f, 0.25f, 0.f), 	//top-right
		cv::Point3f(0.25f, -0.25f, 0.f),	//bottom-right
		cv::Point3f(-0.25f, -0.25f, 0.f),	//bottom-left
		cv::Point3f(-0.25f, 0.25f, 0.f),	//top-left
	}, "Test Retroreflective Square") {}

};



class BBoxDemo : public vs1::VPipeline, public vs1::WeightedSubtraction<vs1::VThreshold::LED::BLUE>, public vs1::Contours {
public:
	BBoxDemo(vs1::VisionServer& server);
	BBoxDemo(const BBoxDemo& other) = delete;

	void process(cv::Mat& io_frame) override;

private:
	cv::Rect boundingbox;

};
class SquareTargetPNP : public vs1::VPipeline, public vs1::WeightedSubtraction<vs1::VThreshold::LED::BLUE>, public vs1::Contours {
public:
	SquareTargetPNP(vs1::VisionServer& server);
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