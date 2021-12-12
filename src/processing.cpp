#include "processing.h"

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
void WSTPipeline::threshold(const cv::Mat& io_frame) {
	if(io_frame.size() != this->resolution) {
		this->resizeBuffers(io_frame.size());
	}

	cv::resize(io_frame, this->buffer, cv::Size(), 1.0/this->scale, 1.0/this->scale);
	cv::split(this->buffer, this->channels);
	cv::addWeighted(this->channels[weighted_array[(size_t)this->color][0]], this->weight, this->channels[weighted_array[(size_t)this->color][1]], this->weight, 0.0, this->binary);
	cv::subtract(this->channels[(size_t)this->color], this->binary, this->binary);
	memcpy_threshold_binary_asm(this->binary.data, this->binary.data, this->binary.size().area(), this->thresh);
}
void WSTPipeline::process(cv::Mat& io_frame, bool output_binary) {}

ContourPipeline::ContourPipeline() {}

size_t ContourPipeline::findContours(const cv::Mat& binary_frame) {
	this->contours.clear();
	cv::findContours(binary_frame, this->contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	return this->contours.size();
}
size_t ContourPipeline::findLargest(const cv::Mat& binary_frame) {
	this->largest = 0.f;
	this->target = -1;

	this->findContours(binary_frame);
	for(size_t i = 0; i < this->contours.size(); i++) {
		this->area = cv::contourArea(this->contours[i]);
		if(this->area > this->largest) {
			this->largest = this->area;
			this->target = i;
		}
	}
	return this->target;
}
size_t ContourPipeline::findLargestGT(const cv::Mat& binary_frame, double area_gt) {
	this->largest = 0.f;
	this->target = -1;

	this->findContours(binary_frame);
	for(size_t i = 0; i < this->contours.size(); i++) {
		this->area = cv::contourArea(this->contours[i]);
		if(this->area > area_gt && this->area > this->largest) {
			this->largest = this->area;
			this->target = i;
		}
	}
	return this->target;
}