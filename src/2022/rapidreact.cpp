#include "rapidreact.h"

#include <algorithm>

void UpperHub::sort() {
	std::sort(
		this->points.begin(),
		this->points.end(),
		[](const cv::Point2f& a, const cv::Point2f& b) { return a.x < b.x; }
	);
	this->matchWorld();
}
void UpperHub::matchWorld() {
	this->world_match.clear();
	for(size_t i = 0; i < this->points.size(); i++) {
		this->world_match.push_back(this->world[i]);
	}
}
void UpperHub::solvePerspective(
	cv::Mat_<float>& tvec, cv::Mat_<float>& rvec,
	cv::InputArray camera_matrix, cv::InputArray camera_coeffs,
	int flags, bool ext_guess
) {
	std::cout << world_match.size() << " : " << points.size() << newline;
	cv::solvePnP(this->world_match, this->points, camera_matrix, camera_coeffs, rvec, tvec, ext_guess, flags);

	this->getTable()->PutNumber("x", tvec[0][0]);
	this->getTable()->PutNumber("y", tvec[1][0]);
	this->getTable()->PutNumber("z", tvec[2][0]);
	this->getTable()->PutNumber("distance", sqrt(pow(tvec[0][0], 2) + pow(tvec[1][0], 2) + pow(tvec[2][0], 2)));
	//this->getTable()->PutNumber("up-down", atan2(tvec[1][0], tvec[2][0])*-180/M_PI);
	//this->getTable()->PutNumber("left-right", atan2(tvec[0][0], tvec[2][0])*180/M_PI);
}