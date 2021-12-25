#include "targets.h"

#include <networktables/NetworkTableInstance.h>

#include <string>
#include <algorithm>
#include <math.h>

template<size_t corners>
Target<corners>::Target(const std::array<cv::Point3f, corners>& world_pts) : 
	world(world_pts), name("Unnamed " + std::to_string(corners) + "-pt target"),
	table(nt::NetworkTableInstance::GetDefault().GetTable("Targets")->GetSubTable(this->name)) {}
template<size_t corners>
Target<corners>::Target(const std::array<cv::Point3f, corners>& world_pts, const char* name) : 
	world(world_pts), name(name), table(nt::NetworkTableInstance::GetDefault().GetTable("Targets")->GetSubTable(name)) {}
template<size_t corners>
Target<corners>::Target(const std::array<cv::Point3f, corners>& world_pts, const std::string& name) : 
	world(world_pts), name(name), table(nt::NetworkTableInstance::GetDefault().GetTable("Targets")->GetSubTable(name)) {}

template<size_t corners>
template<typename num_t>
void Target<corners>::sort(const std::vector<cv::Point_<num_t> >& contour) {
	static_assert(std::is_arithmetic<num_t>::value, "Number type must be arithmetic");
	this->center = findCenter<float, num_t>(contour);
	size_t limit = corners > contour.size() ? contour.size() : corners;
	for(size_t i = 0; i < limit; i++) {
		this->points[i] = contour[i];
	}
	std::sort(
		this->points.begin(), 
		this->points.end(), 
		[this](const cv::Point2f& a, const cv::Point2f& b) {
			this->a = a - this->center;
			this->a = b - this->center;
			return -atan2(this->a.x, this->a.y) < -atan2(this->b.x, this->b.y);	// note that x and y are switched -> clockwise
		}
	);
}
template<size_t corners>
void Target<corners>::rescale(double scale) {
	for(size_t i = 0; i < corners; i++) {
		this->points[i] *= scale;
	}
}
template<size_t corners>
std::array<cv::Point2f, corners> Target<corners>::getRescaled(double scale) const {
	std::array<cv::Point2f, corners> ret;
	for(size_t i = 0; i < corners; i++) {
		ret[i] = this->points[i] * scale;
	}
	return ret;
}

template<size_t corners>
void Target<corners>::solvePerspective(
	cv::Mat_<float>& tvec, cv::Mat_<float>& rvec, 
	cv::InputArray camera_matrix, cv::InputArray camera_coeffs, 
	bool ext_guess, int flags
) {
	cv::solvePnP(this->world, this->points, camera_matrix, camera_coeffs, rvec, tvec, ext_guess, flags);

	this->table->PutNumber("x", tvec[0][0]);
	this->table->PutNumber("y", tvec[1][0]);
	this->table->PutNumber("z", tvec[2][0]);
	this->table->PutNumber("distance", sqrt(pow(tvec[0][0], 2) + pow(tvec[1][0], 2) + pow(tvec[2][0], 2)));
	this->table->PutNumber("up-down", atan2(tvec[1][0], tvec[2][0])*-180/M_PI);
	this->table->PutNumber("left-right", atan2(tvec[0][0], tvec[2][0])*180/M_PI);
}