#include "target.h"

#include <networktables/NetworkTableInstance.h>

#include <algorithm>
#include <math.h>

#include "vision.h"

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
	this->center = ::findCenter<float, num_t>(contour);
	size_t limit = contour.size() < corners ? contour.size() : corners;
	for(size_t i = 0; i < limit; i++) {
		this->points[i] = contour[i];
	}
	std::sort(
		this->points.begin(), 
		this->points.end(), 
		[this](const cv::Point2f& a, const cv::Point2f& b) {
			this->a = a - this->center;
			this->b = b - this->center;
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



template<class target_t, class threshold_t>
TargetSolver<target_t, threshold_t>::TargetSolver(VisionServer& server) :
	VPipeline(server, "TargetSolver Pipeline"), threshold_t(server, this->table) 
{
	this->table->PutBoolean("Show Thresholded", false);
	this->table->PutBoolean("Debug", false);
}
// template<typename target_t, VThreshold::LED color>
// TargetSolver<target_t, color>::TargetSolver(TargetSolver&& other) :
// 	VPipeline(static_cast<VPipeline&&>(other)), 
// 	WeightedSubtraction<color>(static_cast<WeightedSubtraction<color>&>(other)) {}
template<class target_t, class threshold_t>
TargetSolver<target_t, threshold_t>::~TargetSolver() {
	this->table->Delete("Show Thresholded");
	this->table->Delete("Debug");
}

template<class target_t, class threshold_t>
void TargetSolver<target_t, threshold_t>::process(cv::Mat& io_frame, int8_t mode) {

	this->findLargest(this->threshold(io_frame));
	if(this->table->GetBoolean("Show Thresholded", false)) {
		this->fromBinary(io_frame);
	}

	if(this->validTarget()) {
		cv::convexHull(this->getTarget(), this->target_points);
		cv::approxPolyDP(this->target_points, this->target_points, 0.1*cv::arcLength(this->getTarget(), false), true);
		rescale(this->target_points, this->getScale());

		if(this->reference_points.compatible(this->target_points)) {
			this->reference_points.sort(this->target_points);
			this->reference_points.solvePerspective(this->tvec, this->rvec, this->getCameraMatrix(), this->getCameraDistortion());

			this->updateTarget(this->reference_points.getName());
			
			if(this->table->GetBoolean("Debug", false)) {
				cv::projectPoints(this->pose3D, this->rvec, this->tvec, this->getCameraMatrix(), this->getCameraDistortion(), this->pose2D);
				cv::line(io_frame, this->reference_points.getCenter(), this->pose2D[0], cv::Scalar(255, 0, 0), 2);
				for(size_t i = 0; i < this->reference_points.size(); i++) {
					cv::putText(
						io_frame, std::to_string(i), 
						this->reference_points.points[i], 
						cv::FONT_HERSHEY_DUPLEX, 0.7, cv::Scalar(255, 0, 0), 1, cv::LINE_AA
					);
				}
			}
		} else {
			if(this->table->GetBoolean("Debug", false)) {
				for(size_t i = 0; i < this->target_points.size(); i++) {
					cv::putText(
						io_frame, std::to_string(i), 
						this->target_points[i], 
						cv::FONT_HERSHEY_DUPLEX, 0.7, cv::Scalar(255, 0, 0), 1, cv::LINE_AA
					);
				}
				cv::circle(io_frame, ::findCenter(this->target_points), 1, cv::Scalar(255, 255, 0), 2);
			}
		}
	}
	// no targets detected
}