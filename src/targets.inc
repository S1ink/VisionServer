#include "targets.h"

#include <string>
#include <algorithm>
#include <math.h>

template<size_t corners>
Target<corners>::Target(const std::array<cv::Point3f, corners>& world_pts) : world(world_pts) {
	std::string n = "targets/unnamed_" + std::to_string(corners) + "-pt_target";
	this->table = nt::NetworkTableInstance::GetDefault().GetTable("Targets")->GetSubTable(n);
}
template<size_t corners>
Target<corners>::Target(const std::array<cv::Point3f, corners>& world_pts, const char* tname) : 
	world(world_pts), table(nt::NetworkTableInstance::GetDefault().GetTable("Targets")->GetSubTable(tname)) {}
template<size_t corners>
Target<corners>::Target(const std::array<cv::Point3f, corners>& world_pts, const wpi::Twine& tname) : 
	world(world_pts), table(nt::NetworkTableInstance::GetDefault().GetTable("Targets")->GetSubTable(tname)) {}

template<size_t corners>
size_t Target<corners>::size() const {return corners;}
template<size_t corners>
void Target<corners>::reorder(const std::vector<cv::Point2i>& contour) {
	cv::Point2f center = findCenter<float>(contour), buff1, buff2;
	size_t limit = corners > contour.size() ? contour.size() : corners;
	for(size_t i = 0; i < limit; i++) {
		this->points[i] = contour[i];
	}
	std::sort(
		this->points.begin(), 
		this->points.end(), 
		[this, center, &buff1, &buff2](const cv::Point2f& a, const cv::Point2f& b) {
			buff1 = a - center;
			buff2 = b - center;
			return -atan2(buff1.x, buff1.y) < -atan2(buff2.x, buff2.y);	// note that x and y are switched -> clockwise
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
// template<size_t corners>
// void Target<corners>::solvePerspective(
// 	const std::vector<cv::Point>& contour,
// 	cv::OutputArray tvec, cv::OutputArray rvec, 
// 	cv::InputArray camera_matrix, cv::InputArray camera_coeffs, 
// 	bool ext_guess, int flags
// ) {
// 	this->reorder(contour);
// 	this->solvePerspective(tvec, rvec, camera_matrix, camera_coeffs, ext_guess, flags);
// }