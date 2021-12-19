#include "pipelines.h"

#include <algorithm>
#include <math.h>

#include "vision.h"
#include "mem.h"

template<typename target_t, VThreshold::LED color>
TargetSolver<target_t, color>::TargetSolver(VisionServer& server) :
	VPipeline(server, "TargetSolver Pipeline"), WSThreshold<color>(server.getCurrentResolution(), this->table) {}
template<typename target_t, VThreshold::LED color>
void TargetSolver<target_t, color>::process(cv::Mat& io_frame, bool debug) {
	this->threshold(io_frame);
	this->findLargest(this->binary);

	if(debug) {
		cv::cvtColor(this->binary, this->buffer, cv::COLOR_GRAY2BGR, 3);
		cv::resize(this->buffer, io_frame, cv::Size(), this->scale, this->scale, cv::INTER_NEAREST);
	} else {

		if(this->validTarget()) {
			cv::convexHull(this->getTarget(), this->target_points);
			cv::approxPolyDP(this->target_points, this->target_points, 0.1*cv::arcLength(this->getTarget(), false), true);

			rescale(this->target_points, this->scale);
			if(this->reference_points.compatible(this->target_points)) {
				this->reference_points.reorder(this->target_points);
				this->reference_points.solvePerspective(this->tvec, this->rvec, this->getCameraMatrix(), this->getCameraDistortion());

				for(size_t i = 0; i < this->reference_points.points.size(); i++) {
					cv::putText(
						io_frame, std::to_string(i), 
						this->reference_points.points[i], 
						cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 0, 0), 2, cv::LINE_AA
					);
				}
			} else {
				for(size_t i = 0; i < this->target_points.size(); i++) {
					cv::putText(
						io_frame, std::to_string(i), 
						this->target_points[i], 
						cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(0, 0, 255), 2, cv::LINE_AA
					);
				}
			}
			cv::circle(io_frame, ::findCenter(this->target_points), 1, cv::Scalar(255, 255, 0), 2);
		} else {
			cv::putText(
				io_frame, "NO TARGETS DETECTED", 
				cv::Point(io_frame.size().width*0.5 - 192, io_frame.size().height*0.5), 
				cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 255), 2, cv::LINE_AA
			);
		}
	}
}