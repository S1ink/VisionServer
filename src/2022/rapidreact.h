#pragma once

#include <opencv2/opencv.hpp>

#include <vector>
#include <array>

#include "../api/weightedsubtraction.h"
#include "../api/visionserver.h"
#include "../api/processing.h"
#include "../api/target.h"

/**
 * Models 3-5 of the closest 5(or 4) vision strips on the Upper Hub
 * 
 * - The diameter is 4ft,5.375in (427/8 or 53.375 inches) -> radius is 427/16 or 26.6875 inches
 * - Each strip is 5in, with 5.5in separation (along the circumfrance) -> also 2in high
 * - The x and z points are found by:
 * 		x: r*sin(m/r), where r is the radius(26.6875), d is the diameter(53.375), and m is the midpoint of a strip along the circumfrance (-21, -10.5, 0, 10.5, 21) and (-15.75, -5.25, 5.25, 15.75)
 * 		z: r*cos(m/r), where r is the radius(26.6875), d is the diameter(53.375), and m is the midpoint of a strip along the circumfrance (-21, -10.5, 0, 10.5, 21) and (-15.75, -5.25, 5.25, 15.75)
 * 
 * See: https://www.desmos.com/calculator/lmxzqaxwvy
*/
class UpperHub : public Target<0> {
public:
	UpperHub() : Target<0>({}, "Upper-Hub"), points(5), world_match(5) {}
	UpperHub(const UpperHub&) = delete;
	
	inline size_t size() { return this->points.size(); }
	inline bool compatible(const std::vector<cv::Point>& contour) const { return contour.size() > 2 && contour.size() <= 5; }

	template<typename num_t> 
	void sort(const std::vector<cv::Point_<num_t> >& contour);

	//void rescale(double scale);
	//std::vector<cv::Point2f> getRescaled(double scale) const;

	void solvePerspective(
		cv::Mat_<float>& tvec, cv::Mat_<float>& rvec,
		cv::InputArray camera_matrix, cv::InputArray camera_coeffs,
		bool ext_guess = false, int flags = 0
	);

private:
	std::vector<cv::Point2f> points;
	std::vector<cv::Point3f> world_match;
	const std::array<cv::Point3f, 9> world{		// left to right
		cv::Point3f(-18.89895395f, 0.f, 18.84282879f),
		cv::Point3f(-14.85151996f, 0.f, 22.173295f),
		cv::Point3f(-10.23119406f, 0.f, 24.64843452f),
		cv::Point3f(-5.216203596f, 0.f, 26.17276975f),
		cv::Point3f(0.f, 0.f, 26.6875f),
		cv::Point3f(5.216203596f, 0.f, 26.17276975f),
		cv::Point3f(10.23119406f, 0.f, 24.64843452f),
		cv::Point3f(14.85151996f, 0.f, 22.173295f),
		cv::Point3f(18.89895395f, 0.f, 18.84282879f),
	};

	std::array<float, 4> comparisons;

};

template<VThreshold::LED color>
class StripFinder : public VPipeline, public WeightedSubtraction<color>, public Contours {
public:
	StripFinder(VisionServer& server);
	StripFinder(const StripFinder& other) = delete;

	void process(cv::Mat& io_frame, int8_t mode = 0) override;

private:
	std::vector<std::vector<cv::Point2i> > filtered;
	std::vector<cv::Point2i> point_buffer;

	//size_t highest{0};

};

#include "rapidreact.inc"