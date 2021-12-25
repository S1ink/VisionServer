#pragma once

#include <networktables/NetworkTable.h>

#include <opencv2/opencv.hpp>

#include <vector>
#include <array>
#include <string>

#include "vision.h"

template<size_t corners>
class Target {
public:
	Target(const std::array<cv::Point3f, corners>& world_pts);
	Target(const std::array<cv::Point3f, corners>& world_pts, const char* name);
	Target(const std::array<cv::Point3f, corners>& world_pts, const std::string& name);

	std::array<cv::Point2f, corners> points;		// actual points 
	const std::array<cv::Point3f, corners> world;	// world points that relate to the above 

	static inline size_t size() { return corners; }
	inline const std::string& getName() const { return this->name; }
	inline bool compatible(const std::vector<cv::Point>& contour) const { return contour.size() == corners; }

	template<typename num_t> void sort(const std::vector<cv::Point_<num_t> >& contour);

	void rescale(double scale);
	std::array<cv::Point2f, corners> getRescaled(double scale) const;

	void solvePerspective(
		cv::Mat_<float>& tvec, cv::Mat_<float>& rvec, 
		cv::InputArray camera_matrix, cv::InputArray camera_coeffs, 
		bool ext_guess = false, int flags = 0
	);

private:
	const std::string name;
	const std::shared_ptr<nt::NetworkTable> table;

	cv::Point2f center, a, b;
	size_t limit;

};

#include "targets.inc"



class Test6x6 : public Target<4> {
public:
	Test6x6() : Target<4>({		// world points in clockwise order
		cv::Point3f(0.25f, 0.25f, 0.f), 	//top-right
		cv::Point3f(0.25f, -0.25f, 0.f),	//bottom-right
		cv::Point3f(-0.25f, -0.25f, 0.f),	//bottom-left
		cv::Point3f(-0.25f, 0.25f, 0.f),	//top-left
	}, "Test Retroreflective Square") {}

};