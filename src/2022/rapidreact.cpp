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
	//std::cout << world_match.size() << " : " << points.size() << newline;

	// if(tvec[1][0] > 40 && tvec[1][0] < 104) {
	// 	ext_guess = true;
	// }
	cv::solvePnPGeneric(this->world_match, this->points, camera_matrix, camera_coeffs, this->rvecs, this->tvecs, ext_guess, cv::SOLVEPNP_IPPE);

	std::array<double, 2> grade;
	for(size_t i = 0; i < this->tvecs.size(); i++) {
		grade[i] = tvecs[i].at<double>({1,0}) + rvecs[i].at<double>({0,0});
	}
	size_t best = grade[0] > grade[1] ? 0 : 1;
	tvec = this->tvecs[best];
	rvec = this->rvecs[best];

	cv::solvePnPRefineLM(this->world_match, this->points, camera_matrix, camera_coeffs, rvec, tvec);

	//tvec = this->tstart;
	//rvec = this->rstart;
	//cv::solvePnP(this->world_match, this->points, camera_matrix, camera_coeffs, rvec, tvec, ext_guess, cv::SOLVEPNP_ITERATIVE);

	// for(size_t i = 0; i < this->tvecs.size(); i++) {
	// 	std::cout
	// 		<< "TRANSLATION:\n"
	// 		<< this->tvecs[i] << newline
	// 		<< "ROTATION:\n"
	// 		<< this->rvecs[i] << newline
	// 		<< "GRADE: " << grade[i] << "\n\n";
	// 	std::cout.flush();
	// }
	// std::cout << "\n\n";

	this->getTable()->PutNumber("x", tvec[0][0]);
	this->getTable()->PutNumber("y", tvec[1][0]);
	this->getTable()->PutNumber("z", tvec[2][0]);
	this->getTable()->PutNumber("distance", sqrt(pow(tvec[0][0], 2) + pow(tvec[1][0], 2) + pow(tvec[2][0], 2)));
	//this->getTable()->PutNumber("up-down", atan2(tvec[1][0], tvec[2][0])*-180/M_PI);
	//this->getTable()->PutNumber("left-right", atan2(tvec[0][0], tvec[2][0])*180/M_PI);
}