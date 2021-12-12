#include "pipelines.h"
#include "vision.h"

const std::array<std::array<uint8_t, 2>, 3> WSTPipeline::weighted_array = {
	std::array<uint8_t, 2>{(uint8_t)PColor::GREEN, (uint8_t)PColor::RED},
	std::array<uint8_t, 2>{(uint8_t)PColor::BLUE, (uint8_t)PColor::RED},
	std::array<uint8_t, 2>{(uint8_t)PColor::BLUE, (uint8_t)PColor::GREEN},
};

WSTPipeline::WSTPipeline(VisionServer* server, std::shared_ptr<nt::NetworkTable> table) : PipelineBase(server) {
	addNetTableVar(this->weight, "Weight", table);
	addNetTableVar(this->thresh, "Threshold", table);
	//addNetTableVar((uint8_t)this->color, "Color (B:0, G:1, R:2", table);

	this->resizeBuffers(this->env->getCurrentResolution());
}

void WSTPipeline::resizeBuffers(cv::Size size) {
	this->buffer = cv::Mat(size/this->scale, CV_8UC3);
	this->binary = cv::Mat(size/this->scale, CV_8UC1);
	for(size_t i = 0; i < this->channels.size(); i++) {
		channels[i] = cv::Mat(size/this->scale, CV_8UC1);
	}
	this->resolution = size;
}
void WSTPipeline::threshold(cv::Mat& io_frame) {
	if(io_frame.size() != this->resolution) {
		this->resizeBuffers(io_frame.size());
	}

	cv::resize(io_frame, this->buffer, cv::Size(), 1.0/this->scale, 1.0/this->scale);
	cv::split(this->buffer, this->channels);
	cv::addWeighted(this->channels[weighted_array[(size_t)this->color][0]], this->weight, this->channels[weighted_array[(size_t)this->color][1]], this->weight, 0.0, this->binary);
	cv::subtract(this->channels[(size_t)this->color], this->binary, this->binary);
	memcpy_threshold_binary_asm(this->binary.data, this->binary.data, this->binary.size().area(), this->thresh);
}

BBoxDemo::BBoxDemo(VisionServer* server) : WSTPipeline(server, this->table) {}

void BBoxDemo::process(cv::Mat& io_frame, bool output_binary) {
	this->threshold(io_frame);
	if(output_binary) {
		cv::cvtColor(this->binary, this->buffer, cv::COLOR_GRAY2BGR, 3);
		cv::resize(this->buffer, io_frame, cv::Size(), this->scale, this->scale, cv::INTER_NEAREST);
	} else {
		this->contours.clear();
		this->t_contour.clear();
		this->largest = 0.f;
		this->target = -1;

		cv::findContours(this->binary, this->contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

		for(size_t i = 0; i < this->contours.size(); i++) {
			this->area = cv::contourArea(this->contours[i]);
			if(this->area > this->largest) {
				this->largest = this->area;
				this->target = i;
			}
		}

		if(this->target >= 0) {
			this->boundingbox = cv::boundingRect(this->contours[this->target]);
			cv::rectangle(io_frame, this->boundingbox.tl()*(int)this->scale, this->boundingbox.br()*(int)this->scale, cv::Scalar(255, 0, 0), 2);
		} else {
			cv::putText(
				io_frame, "NO TARGETS DETECTED", 
				cv::Point(io_frame.size().width*0.5 - 192, io_frame.size().height*0.5), 
				cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 255), 2, cv::LINE_AA
			);
		}
	}
}



TestPipeline::TestPipeline(VisionServer* server) : PipelineBase(server) {
	addNetTableVar(this->weight, "Weight (Red and Green)", this->table);
	addNetTableVar(this->threshold, "Threshold (For weighted)", this->table);
	//addNetTableVar(this->options[0], "Morphological Operations", this->table);

	this->resizeBuffers(this->env->getCurrentResolution());
}

void TestPipeline::resizeBuffers(cv::Size size) {
	this->buffer = cv::Mat(size/this->scale, CV_8UC3);
	this->binary = cv::Mat(size/this->scale, CV_8UC1);
	for(size_t i = 0; i < this->channels.size(); i++) {
		channels[i] = cv::Mat(size/this->scale, CV_8UC1);
	}
}

cv::Point2d TestPipeline::SquareTarget::findCenter() {
	double x = 0, y = 0;
	for(size_t i = 0; i < this->data.array.size(); i++) {
		x += this->data.array[i].x;
		y += this->data.array[i].y;
	}
	x /= this->data.array.size();
	y /= this->data.array.size();
	return cv::Point2d(x, y);
}
void TestPipeline::SquareTarget::sortPoints(const std::vector<cv::Point>& points) {
	cv::Point2d center = ::findCenter<double>(points);
	bool top = false, bottom = false;
	for(size_t i = 0; i < points.size(); i++) {
		if(points[i].y < center.y) {
			if(top) {
				if(points[i].x < this->data.points.top_left.x) {
					this->data.points.top_right = this->data.points.top_left;
					this->data.points.top_left = points[i];
				} else {
					this->data.points.top_right = points[i];
				}
			} else {
				this->data.points.top_left = points[i];
				top = true;
			}
		} else {
			if(bottom) {
				if(points[i].x < this->data.points.bottom_left.x) {
					this->data.points.bottom_right = this->data.points.bottom_left;
					this->data.points.bottom_left = points[i];
				} else {
					this->data.points.bottom_right = points[i];
				}
			} else {
				this->data.points.bottom_left = points[i];
				bottom = true;
			}
		}
	}
}

void TestPipeline::SquareTarget::scaleUp(size_t scale) {
	for(size_t i = 0; i < this->data.array.size(); i++) {
		this->data.array[i] *= (int)scale;
	}
}
void TestPipeline::SquareTarget::scaleDown(size_t scale) {
	for(size_t i = 0; i < this->data.array.size(); i++) {
		this->data.array[i] *= (double)(1.0/scale);
	}
}

void TestPipeline::process(cv::Mat& io_frame, bool show_binary) {

	if(cmpSize(io_frame.size())) {
		this->resizeBuffers(io_frame.size());
	}

	cv::resize(io_frame, this->buffer, cv::Size(), 1.0/this->scale, 1.0/this->scale);
	cv::split(this->buffer, this->channels);
	cv::addWeighted(this->channels[1], this->weight, this->channels[2], this->weight, 0.0, this->binary);
	cv::subtract(this->channels[0], this->binary, this->binary);
	memcpy_threshold_binary_asm(this->binary.data, this->binary.data, this->binary.size().area(), this->threshold);

	// if(options[0]) {
	// 	cv::erode(this->binary, this->binary, cv::getStructuringElement(cv::MORPH_ERODE, cv::Size(3, 3)));
	// 	cv::dilate(this->binary, this->binary, cv::getStructuringElement(cv::MORPH_DILATE, cv::Size(3, 3)));
	// }

	if(show_binary) {
		cv::cvtColor(this->binary, this->buffer, cv::COLOR_GRAY2BGR, 3);
		cv::resize(this->buffer, io_frame, cv::Size(), this->scale, this->scale, cv::INTER_NEAREST);
	} else {
		this->contours.clear();
		this->t_contour.clear();
		this->largest = 0.f;
		this->target = -1;

		cv::findContours(this->binary, this->contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

		for(size_t i = 0; i < this->contours.size(); i++) {
			this->area = cv::contourArea(this->contours[i]);
			if(this->area > this->largest) {
				this->largest = this->area;
				this->target = i;
			}
		}

		if(this->target >= 0) {
			// bounding box demo
			// this->boundingbox = cv::boundingRect(this->contours[this->target]);
			// cv::Point2i tl = this->boundingbox.tl(), br = this->boundingbox.br();
			// cv::rectangle(io_frame, cv::Point2i(tl.x*this->scale, tl.y*this->scale), cv::Point(br.x*this->scale, br.y*this->scale), cv::Scalar(255, 0, 0), 2);

			// simplify points using convextHull and approxPolyDP
			cv::convexHull(this->contours[this->target], this->t_contour);
			cv::approxPolyDP(this->t_contour, this->t_contour, 0.1 * cv::arcLength(this->contours[this->target], false), true);
			// for(size_t i = 0; i < this->t_contour.size(); i++) {
			// 	cv::circle(io_frame, this->t_contour[i]*(int)this->scale, 1, cv::Scalar(255, 0, 0), 2);
			// }

			if(this->t_contour.size() == this->world_corners.size()) {
				this->image_corners.sortPoints(this->t_contour);
				this->image_corners.scaleUp(this->scale);
				cv::solvePnP(this->world_corners, this->image_corners.data.array, this->camera_matrix, this->distortion, this->rvec, this->tvec);

				float 
					x = this->tvec[0][0],
					y = this->tvec[1][0],
					z = this->tvec[2][0];
				double distance = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
				double tangent_lr = atan2(x, z)*180/M_PI;	// angle to turn left/right in order to be inline
				double tangent_ud = atan2(y, z)*-180/M_PI;	// angle to turn up/down in order to be inline

				cv::putText(
					io_frame, "DISTANCE: " + std::to_string(distance) + "ft", 
					cv::Point(io_frame.size().width*0.65, 20), 
					cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 0, 255), 1, cv::LINE_AA
				);
				cv::putText(
					io_frame, "Tangent L/R: " + std::to_string(tangent_lr) + "*", 
					cv::Point(io_frame.size().width*0.73, 40), 
					cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(255, 0, 255), 1, cv::LINE_AA
				);
				cv::putText(
					io_frame, "Tangent U/D: " + std::to_string(tangent_ud) + "*", 
					cv::Point(io_frame.size().width*0.73, 60), 
					cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(255, 0, 255), 1, cv::LINE_AA
				);
				cv::putText(
					io_frame, "X: " + std::to_string(x) + "ft", 
					cv::Point(io_frame.size().width*0.84, 80), 
					cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(255, 0, 255), 1, cv::LINE_AA
				);
				cv::putText(
					io_frame, "Y: " + std::to_string(y) + "ft", 
					cv::Point(io_frame.size().width*0.84, 100), 
					cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(255, 0, 255), 1, cv::LINE_AA
				);
				cv::putText(
					io_frame, "Z: " + std::to_string(z) + "ft", 
					cv::Point(io_frame.size().width*0.84, 120), 
					cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(255, 0, 255), 1, cv::LINE_AA
				);

				//cv::Rodrigues(this->rvec, this->rmat);

				cv::projectPoints(this->perpendicular3d, this->rvec, this->tvec, this->camera_matrix, this->distortion, this->perpendicular2d);
				for(size_t i = 0; i < this->perpendicular2d.size(); i++) {
					cv::line(io_frame, this->perpendicular2d[i], this->image_corners.data.array[i], cv::Scalar(255, 0, 0), 1);
				}
				cv::line(io_frame, this->perpendicular2d[0], this->perpendicular2d[1], cv::Scalar(255, 0, 0), 1);
				cv::line(io_frame, this->perpendicular2d[1], this->perpendicular2d[3], cv::Scalar(255, 0, 0), 1);
				cv::line(io_frame, this->perpendicular2d[2], this->perpendicular2d[0], cv::Scalar(255, 0, 0), 1);
				cv::line(io_frame, this->perpendicular2d[3], this->perpendicular2d[2], cv::Scalar(255, 0, 0), 1);

				for(size_t i = 0; i < this->image_corners.data.array.size(); i++) {
					//cv::circle(io_frame, this->image_corners.data.array[i]*(int)this->scale, 1, cv::Scalar(255, 0, 0), 2);
					cv::putText(
						io_frame, std::to_string(i), 
						this->image_corners.data.array[i], 
						cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 0, 0), 2, cv::LINE_AA
					);
				}
			} else {
				for(size_t i = 0; i < this->t_contour.size(); i++) {
					//cv::circle(io_frame, this->t_contour[i]*(int)this->scale, 1, cv::Scalar(0, 0, 255), 2);
					cv::putText(
						io_frame, std::to_string(i), 
						this->t_contour[i]*(int)this->scale, 
						cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(0, 0, 255), 2, cv::LINE_AA
					);
				}
			}
			cv::circle(io_frame, ::findCenter(this->t_contour)*(int)this->scale, 1, cv::Scalar(255, 255, 0), 2);
		} else {
			cv::putText(
				io_frame, "NO TARGETS DETECTED", 
				cv::Point(io_frame.size().width*0.2, io_frame.size().height*0.5), 
				cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 255), 2, cv::LINE_AA
			);
		}
	}
}