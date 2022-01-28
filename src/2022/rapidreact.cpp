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

//template<typename num_t>
void Cargo::sort(CargoOutline outline) {
	this->points = {
		cv::Point2f(outline.center.x - outline.radius, outline.center.y),
		cv::Point2f(outline.center.x, outline.center.y-outline.radius),
		cv::Point2f(outline.center.x + outline.radius, outline.center.y)
	};
}

CargoFinder::CargoFinder(VisionServer& server) :
	VPipeline(server, "Cargo Finder"), WeightedSubtraction<LED::RED>(server, this->table), red(*this), blue(*this)
{
	this->table->PutString("Show Thresholded", "None");
	// add something to control which color is processed
}

void CargoFinder::process(cv::Mat& io_frame, int8_t mode) {
	if(io_frame.size() != this->buffer.size()*(int)this->scale) {
		this->resizeBuffers(io_frame.size());
	}
	cv::resize(io_frame, this->buffer, cv::Size(), 1.0/this->scale, 1.0/this->scale);
	cv::split(this->buffer, this->channels);

	this->filtered.clear();
	this->red.threshold();
	if(this->table->GetString("Show Thresholded", "None") == "Red" || this->table->GetString("Show Thresholded", "None") == "Both") {
		this->fromBinary(io_frame);
	}
	this->blue.threshold();
	if(this->table->GetString("Show Thresholded", "None") == "Blue") {
		this->fromBinary(io_frame);
	} else if(this->table->GetString("Show Thresholded", "None") == "Both") {
		cv::cvtColor(this->binary, this->buffer, cv::COLOR_GRAY2BGR, 3);
		cv::bitwise_or(io_frame, this->buffer, io_frame);
	}

	//this->balls.clear();
	for(size_t i = 0; i < this->filtered.size(); i++) {
		cv::circle(io_frame, this->filtered[i].center, this->filtered[i].radius, (this->filtered[i].color == CargoColor::RED ? cv::Scalar(0, 255, 255) : cv::Scalar(255, 255, 0)), 2);
		//this->balls.emplace_back(i);
		//this->balls.back().sort(this->filtered[i]);

	}
}
void CargoFinder::RedFinder::threshold() {
	cv::addWeighted(env->channels[0], 1.0, env->channels[1], 1.0, 0.0, env->binary);
	cv::subtract(env->channels[2], env->binary, env->binary);
	cv::minMaxIdx(env->binary, nullptr, &env->max_val);
	memcpy_threshold_asm(env->binary.data, env->binary.data, env->binary.size().area(), env->max_val*0.2);

	this->findContours(env->binary);
	this->filtered.clear();

	for(size_t i = 0; i < this->contours.size(); i++) {
		cv::minEnclosingCircle(this->contours[i], env->outline_buffer.center, env->outline_buffer.radius);
		cv::convexHull(this->contours[i], env->point_buffer);
		if(cv::contourArea(env->point_buffer)/(CV_PI * pow(env->outline_buffer.radius, 2)) > 0.8) {
			env->outline_buffer.color = CargoColor::RED;
			env->filtered.push_back(env->outline_buffer);
		}
	}
}
void CargoFinder::BlueFinder::threshold() {
	cv::addWeighted(env->channels[1], 0.2, env->channels[2], 1.0, 0.0, env->binary);
	cv::subtract(env->channels[0], env->binary, env->binary);
	cv::minMaxIdx(env->binary, nullptr, &env->max_val);
	memcpy_threshold_asm(env->binary.data, env->binary.data, env->binary.size().area(), env->max_val*0.2);

	this->findContours(env->binary);
	this->filtered.clear();

	for(size_t i = 0; i < this->contours.size(); i++) {
		cv::minEnclosingCircle(this->contours[i], env->outline_buffer.center, env->outline_buffer.radius);
		cv::convexHull(this->contours[i], env->point_buffer);
		if(cv::contourArea(env->point_buffer)/(CV_PI * pow(env->outline_buffer.radius, 2)) > 0.8) {
			env->outline_buffer.color = CargoColor::BLUE;
			env->filtered.push_back(env->outline_buffer);
		}
	}
	
}