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

	//cv::Rodrigues(rvec, this->rmat);
	//cv::transpose(this->rmat, this->rmat);
	//this->pzero_world = this->rmat * (-tvec);

	this->getTable()->PutNumber("x", tvec[0][0]);
	this->getTable()->PutNumber("y", tvec[1][0]);
	this->getTable()->PutNumber("z", tvec[2][0]);
	this->getTable()->PutNumber("distance", sqrt(pow(tvec[0][0], 2) + pow(tvec[1][0], 2) + pow(tvec[2][0], 2)));
	//this->getTable()->PutNumber("up-down", atan2(tvec[1][0], tvec[2][0])*-180/M_PI);
	this->getTable()->PutNumber("left-right", acos(tvec[2][0]/(sqrt(pow(tvec[0][0], 2) + pow(tvec[2][0], 2)))) * 180 / CV_PI * sgn(tvec[0][0]));
}

//template<typename num_t>
void Cargo::sort(CargoOutline outline) {
	this->points = {
		cv::Point2f(outline.center.x - outline.radius, outline.center.y),
		cv::Point2f(outline.center.x, outline.center.y - outline.radius),
		cv::Point2f(outline.center.x + outline.radius, outline.center.y),
		cv::Point2f(outline.center.x, outline.center.y + outline.radius)
	};
}
#ifdef TENNIS_DEMO
void Tennis::sort(CargoOutline outline) {
	this->points = {
		cv::Point2f(outline.center.x - outline.radius, outline.center.y),
		cv::Point2f(outline.center.x, outline.center.y - outline.radius),
		cv::Point2f(outline.center.x + outline.radius, outline.center.y),
		cv::Point2f(outline.center.x, outline.center.y + outline.radius)
	};
}
#endif

CargoFinder::CargoFinder(VisionServer& server) :
	VPipeline(server, "Cargo Pipeline"), red(server, *this), blue(server, *this)
#ifdef TENNIS_DEMO
	, normal(server, *this)
#endif
{
	this->table->PutBoolean("Process Red", true);
	this->table->PutBoolean("Process Blue", true);
	this->table->PutBoolean("Show Threshold", false);
	this->table->PutBoolean("Show Contours", false);
	this->table->PutNumber("Scaling", 1.0);
	this->table->GetEntry("Scaling").AddListener(
		[this](const nt::EntryNotification& event){
			if(event.value->IsDouble()) {
				this->red.scale = this->blue.scale = (event.value->GetDouble() > 0 ? event.value->GetDouble() : 1U);
#ifdef TENNIS_DEMO
				this->normal.scale = this->red.scale;
#endif
			}
		},
		NT_NOTIFY_IMMEDIATE | NT_NOTIFY_NEW | NT_NOTIFY_UPDATE
	);
}

void CargoFinder::process(cv::Mat& io_frame, int8_t mode) {
	bool do_red = this->table->GetBoolean("Process Red", true),
		do_blue = this->table->GetBoolean("Process Blue", true),
		show_bin = this->table->GetBoolean("Show Threshold", false);

	this->red.resetTargetIdx();		// reset target idx so that if a color is disabled while in a "valid" state it doesn't stay like that
	this->blue.resetTargetIdx();

	if(io_frame.size() != this->red.buffer.size()*(int)this->red.scale && do_red) { this->red.resizeBuffers(io_frame.size()); }
	if(io_frame.size() != this->blue.buffer.size()*(int)this->blue.scale && do_blue) { this->blue.resizeBuffers(io_frame.size()); }
#ifdef TENNIS_DEMO
	if(io_frame.size() != this->normal.buffer.size()*(int)this->normal.scale) { this->normal.resizeBuffers(io_frame.size()); }

	this->normal.resetTargetIdx();
#endif

	cv::resize(io_frame, this->red.buffer, cv::Size(), 1.0/this->red.scale, 1.0/this->red.scale);
	cv::split(this->red.buffer, this->red.channels);

	if(do_red) { this->red.launchThresholding(io_frame, this->red.channels); } 
	else if(show_bin) { this->red.buffer = cv::Mat::zeros(io_frame.size(), CV_8UC3); }
	if(do_blue) { this->blue.launchThresholding(io_frame, this->red.channels); } 
	else if(show_bin) { this->blue.buffer = cv::Mat::zeros(io_frame.size(), CV_8UC3); }
#ifdef TENNIS_DEMO
	this->normal.launchThresholding(io_frame, this->red.channels);
	this->normal.join();
#endif
	this->red.join();
	this->blue.join();

	//this->red.threshold(io_frame, this->red.channels);
	//this->blue.threshold(io_frame, this->red.channels);

	if(show_bin) {
		cv::bitwise_or(this->red.buffer, this->blue.buffer, io_frame);
#ifdef TENNIS_DEMO
		cv::bitwise_or(this->normal.buffer, io_frame, io_frame);
#endif
	}

	if(this->red.validTarget() && do_red) {
		this->red_c.sort(this->red.filtered[this->red.getTargetIdx()]);
		this->red_c.solvePerspective(this->tvec, this->rvec, this->getCameraMatrix(), this->getCameraDistortion());
		if(!this->blue.validTarget()) {
			this->updateTarget(this->red_c.getName());
		}
	}
	if(this->blue.validTarget() && do_blue) {
		this->blue_c.sort(this->blue.filtered[this->blue.getTargetIdx()]);
		this->blue_c.solvePerspective(this->tvec, this->rvec, this->getCameraMatrix(), this->getCameraDistortion());
		if(!this->red.validTarget()) {
			this->updateTarget(this->blue_c.getName());
		}
	}
#ifdef TENNIS_DEMO
	if(this->normal.validTarget()) {
		this->tennis.sort(this->normal.filtered[this->normal.getTargetIdx()]);
		this->tennis.solvePerspective(this->tvec, this->rvec, this->getCameraMatrix(), this->getCameraDistortion());
	}
#endif

}