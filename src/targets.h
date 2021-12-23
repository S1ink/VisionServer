#pragma once

#include <networktables/NetworkTableInstance.h>
#include <networktables/NetworkTable.h>
#include <wpi/Twine.h>

#include <opencv2/opencv.hpp>

#include <vector>
#include <array>
#include <string>
#include <unordered_map>

#include "vision.h"

template<size_t corners>
class Target {
public:
	Target(const std::array<cv::Point3f, corners>& world_pts);
	Target(const std::array<cv::Point3f, corners>& world_pts, const char* name);
	Target(const std::array<cv::Point3f, corners>& world_pts, const std::string& name);

	std::array<cv::Point2f, corners> points;		// actual points 
	const std::array<cv::Point3f, corners> world;	// world points that relate to the above 

	size_t size() const;
	const std::string& getName();
	inline bool compatible(const std::vector<cv::Point>& contour) const {
		return contour.size() == corners;
	}

	virtual void reorder(const std::vector<cv::Point2i>& contour);
	void rescale(double scale);	// multiplies points (x and y) by scale for all points
	std::array<cv::Point2f, corners> getRescaled(double scale) const;	// returns rescaled array of points, does not alter internal array

	void solvePerspective(	// solves perspective and outputs position to ntables
		cv::Mat_<float>& tvec, cv::Mat_<float>& rvec, 
		cv::InputArray camera_matrix, cv::InputArray camera_coeffs, 
		bool ext_guess = false, int flags = 0
	);
	// virtual void solvePerspective(	// reorders 'contour' points then solves perspective like above
	// 	const std::vector<cv::Point>& contour,
	// 	cv::OutputArray tvec, cv::OutputArray rvec, 
	// 	cv::InputArray camera_matrix, cv::InputArray camera_coeffs, 
	// 	bool ext_guess = false, int flags = 0
	// );

private:
	const std::string name;
	const std::shared_ptr<nt::NetworkTable> table;

};

#include "targets.inc"



class TestingSquare : public Target<4> {
public:
	TestingSquare() : Target<4>({		// world points in clockwise order
		cv::Point3f(0.25f, 0.25f, 0.f), 	//top-right
		cv::Point3f(0.25f, -0.25f, 0.f),	//bottom-right
		cv::Point3f(-0.25f, -0.25f, 0.f),	//bottom-left
		cv::Point3f(-0.25f, 0.25f, 0.f),	//top-left
	}, "Test Retroreflective Square") {}

	void reorder(const std::vector<cv::Point2i>& contour) override;	// sorts points to be clockwise (match the world points)
private:
	cv::Point2f center, a, b;
	size_t limit;
};