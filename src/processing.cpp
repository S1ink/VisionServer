#include "processing.h"

size_t Contours::findContours(const cv::Mat& binary_frame, int mode, int method) {
	this->contours.clear();
	cv::findContours(binary_frame, this->contours, mode, method);
	return this->contours.size();
}
size_t Contours::findLargest(const cv::Mat& binary_frame, int mode, int method) {
	this->area_largest = 0.f;
	this->target_idx = -1;

	this->findContours(binary_frame, mode, method);
	for(size_t i = 0; i < this->contours.size(); i++) {
		this->area_buff = cv::contourArea(this->contours[i]);
		if(this->area_buff > this->area_largest) {
			this->area_largest = this->area_buff;
			this->target_idx = i;
		}
	}
	return this->target_idx;
}
size_t Contours::findLargestThreshold(const cv::Mat& binary_frame, double area, int mode, int method) {
	this->area_largest = 0.f;
	this->target_idx = -1;

	this->findContours(binary_frame, mode, method);
	for(size_t i = 0; i < this->contours.size(); i++) {
		this->area_buff = cv::contourArea(this->contours[i]);
		if(this->area_buff > area && this->area_buff > this->area_largest) {
			this->area_largest = this->area_buff;
			this->target_idx = i;
		}
	}
	return this->target_idx;
}
size_t Contours::getTargetIdx() const {
	return this->target_idx;
}
const std::vector<cv::Point2i>& Contours::getTarget() const {
	return this->contours[this->target_idx];
}