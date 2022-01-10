#pragma once

#include <opencv2/opencv.hpp>

#include "api/weightedsubtraction.h"
#include "api/visionserver.h"
#include "api/processing.h"
#include "api/target.h"

class SingleStrip : Target<4> {
public:
	SingleStrip() : Target<4>({
		cv::Point3f(2.5f, 1.f, 0.f),	// in inches
		cv::Point3f(2.5f, -1.f, 0.f),
		cv::Point3f(-2.5f, -1.f, 0.f),
		cv::Point3f(-2.5f, 1.f, 0.f)
	}, "Upper-Hub single strip") {}

};
class DoubleStrip : Target<8> {
public:
	DoubleStrip() : Target<8>({
		cv::Point3f(2.75f, 1.f, 0.f),	// actually it's a circle so there would be some depth -> how much though?
		cv::Point3f(7.75f, 1.f, 0.f),
		cv::Point3f(7.75f, -1.f, 0.f),
		cv::Point3f(2.75f, -1.f, 0.f),
		cv::Point3f(-2.75f, -1.f, 0.f),
		cv::Point3f(-7.75f, -1.f, 0.f),
		cv::Point3f(-7.75f, 1.f, 0.f),
		cv::Point3f(-2.75f, 1.f, 0.f)
	}, "Upper-Hub double strip") {}

};