#include "testing.h"


using namespace vs1;

BBoxDemo::BBoxDemo(VisionServer& server) : 
	VPipeline(server, "BoundingBox Demo Pipeline"), WeightedSubtraction(server, this->table) 
{
	// add networktable entry for threshold
}
void BBoxDemo::process(cv::Mat& io_frame) {

	this->findLargest(this->threshold(io_frame));

	if(this->validTarget()) {
		this->boundingbox = cv::boundingRect(this->getTarget());
		cv::rectangle(io_frame, this->boundingbox.tl()*(int)this->scale, this->boundingbox.br()*(int)this->scale, cv::Scalar(255, 0, 0), 2);
	} else {
		cv::putText(
			io_frame, "NO TARGETS DETECTED", 
			cv::Point(io_frame.size().width*0.5 - 192, io_frame.size().height*0.5), 
			cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 255), 2, cv::LINE_AA
		);
	}
}

SquareTargetPNP::SquareTargetPNP(VisionServer& server) : 
	VPipeline(server, "Square Target Pipeline"), WeightedSubtraction(server, this->table) {}

void SquareTargetPNP::process(cv::Mat& io_frame) {
	this->threshold(io_frame);
	this->findLargest(this->binary);

	if(this->validTarget()) {
		cv::convexHull(this->getTarget(), this->target_points);
		cv::approxPolyDP(this->target_points, this->target_points, 0.1*cv::arcLength(this->getTarget(), false), true);
		rescale(this->target_points, this->scale);

		if(this->reference_points.compatible(this->target_points)) {
			this->reference_points.sort(this->target_points);
			//this->reference_points.rescale(this->scale);
			//cv::solvePnP(this->reference_points.world, this->reference_points.points, this->getCameraMatrix(), this->getCameraDistortion(), this->rvec, this->tvec);
			this->reference_points.solvePerspective(this->tvec, this->rvec, this->getCameraMatrix(), this->getCameraDistortion());

			this->updateTarget(this->reference_points.getName());

			// this will go in VisionServer
			// float 
			// 	x = this->tvec[0][0],
			// 	y = this->tvec[1][0],
			// 	z = this->tvec[2][0];
			// double distance = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
			// double tangent_lr = atan2(x, z)*180/M_PI;	// angle to turn left/right in order to be inline
			// double tangent_ud = atan2(y, z)*-180/M_PI;	// angle to turn up/down in order to be inline

			// Position& p = Position::Get();
			// p.setDistance(distance);
			// p.setThetaUD(tangent_ud);
			// p.setThetaLR(tangent_lr);
			// p.setPos(x, y, z);

			// cv::putText(
			// 	io_frame, "DISTANCE: " + std::to_string(distance) + "ft", 
			// 	cv::Point(io_frame.size().width*0.65, 20), 
			// 	cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 0, 255), 1, cv::LINE_AA
			// );
			// cv::putText(
			// 	io_frame, "Tangent L/R: " + std::to_string(tangent_lr) + "*", 
			// 	cv::Point(io_frame.size().width*0.73, 40), 
			// 	cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(255, 0, 255), 1, cv::LINE_AA
			// );
			// cv::putText(
			// 	io_frame, "Tangent U/D: " + std::to_string(tangent_ud) + "*", 
			// 	cv::Point(io_frame.size().width*0.73, 60), 
			// 	cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(255, 0, 255), 1, cv::LINE_AA
			// );
			// cv::putText(
			// 	io_frame, "X: " + std::to_string(x) + "ft", 
			// 	cv::Point(io_frame.size().width*0.84, 80), 
			// 	cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(255, 0, 255), 1, cv::LINE_AA
			// );
			// cv::putText(
			// 	io_frame, "Y: " + std::to_string(y) + "ft", 
			// 	cv::Point(io_frame.size().width*0.84, 100), 
			// 	cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(255, 0, 255), 1, cv::LINE_AA
			// );
			// cv::putText(
			// 	io_frame, "Z: " + std::to_string(z) + "ft", 
			// 	cv::Point(io_frame.size().width*0.84, 120), 
			// 	cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(255, 0, 255), 1, cv::LINE_AA
			// );

			cv::projectPoints(this->projection3d, this->rvec, this->tvec, this->getCameraMatrix(), this->getCameraDistortion(), this->projection2d);
			for(size_t i = 0; i < this->projection2d.size(); i++) {
				cv::line(io_frame, this->projection2d[i], this->reference_points.points[i], cv::Scalar(255, 0, 0), 1);
			}
			cv::line(io_frame, this->projection2d[0], this->projection2d[1], cv::Scalar(255, 0, 0), 1);
			cv::line(io_frame, this->projection2d[1], this->projection2d[2], cv::Scalar(255, 0, 0), 1);
			cv::line(io_frame, this->projection2d[2], this->projection2d[3], cv::Scalar(255, 0, 0), 1);
			cv::line(io_frame, this->projection2d[3], this->projection2d[0], cv::Scalar(255, 0, 0), 1);

			for(size_t i = 0; i < this->reference_points.points.size(); i++) {
				//cv::circle(io_frame, this->image_corners.data.array[i]*(int)this->scale, 1, cv::Scalar(255, 0, 0), 2);
				cv::putText(
					io_frame, std::to_string(i), 
					this->reference_points.points[i], 
					cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 0, 0), 2, cv::LINE_AA
				);
			}
		} else {
			for(size_t i = 0; i < this->target_points.size(); i++) {
				//cv::circle(io_frame, this->t_contour[i]*(int)this->scale, 1, cv::Scalar(0, 0, 255), 2);
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