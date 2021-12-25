#include "pipelines.h"

#include <algorithm>
#include <math.h>

#include "vision.h"

template<class target_t, VThreshold::LED color>
TargetSolver<target_t, color>::TargetSolver(VisionServer& server) :
	VPipeline(server, "TargetSolver Pipeline"), WeightedSubtraction<color>(server.getCurrentResolution(), this->table) {}
// template<typename target_t, VThreshold::LED color>
// TargetSolver<target_t, color>::TargetSolver(TargetSolver&& other) :
// 	VPipeline(static_cast<VPipeline&&>(other)), 
// 	WeightedSubtraction<color>(static_cast<WeightedSubtraction<color>&>(other)) {}

template<class target_t, VThreshold::LED color>
void TargetSolver<target_t, color>::process(cv::Mat& io_frame, int8_t mode) {

	this->threshold(io_frame);
	this->findLargest(this->binary);

	if(this->validTarget()) {
		cv::convexHull(this->getTarget(), this->target_points);
		cv::approxPolyDP(this->target_points, this->target_points, 0.1*cv::arcLength(this->getTarget(), false), true);
		rescale(this->target_points, this->scale);

		if(this->reference_points.compatible(this->target_points)) {
			this->reference_points.sort(this->target_points);
			this->reference_points.solvePerspective(this->tvec, this->rvec, this->getCameraMatrix(), this->getCameraDistortion());

			this->updateTarget(this->reference_points.getName());
		}
	}
	// no targets detected
}