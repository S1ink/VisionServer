#include "processing.h"

size_t ContourPipe::findContours(const cv::Mat& binary_frame) {
	this->contours.clear();
	cv::findContours(binary_frame, this->contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	return this->contours.size();
}
size_t ContourPipe::findLargest(const cv::Mat& binary_frame) {
	this->area_largest = 0.f;
	this->target_idx = -1;

	this->findContours(binary_frame);
	for(size_t i = 0; i < this->contours.size(); i++) {
		this->area_buff = cv::contourArea(this->contours[i]);
		if(this->area_buff > this->area_largest) {
			this->area_largest = this->area_buff;
			this->target_idx = i;
		}
	}
	return this->target_idx;
}
size_t ContourPipe::findLargest_A(const cv::Mat& binary_frame, double area) {
	this->area_largest = 0.f;
	this->target_idx = -1;

	this->findContours(binary_frame);
	for(size_t i = 0; i < this->contours.size(); i++) {
		this->area_buff = cv::contourArea(this->contours[i]);
		if(this->area_buff > area && this->area_buff > this->area_largest) {
			this->area_largest = this->area_buff;
			this->target_idx = i;
		}
	}
	return this->target_idx;
}
size_t ContourPipe::getTargetIdx() const {
	return this->target_idx;
}
const std::vector<cv::Point2i>& ContourPipe::getTarget() const {
	return this->contours[this->target_idx];
}