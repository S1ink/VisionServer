// #include "processing.h"

// size_t ContourPipe::findContours(const cv::Mat& binary_frame) {
// 	this->contours.clear();
// 	cv::findContours(binary_frame, this->contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
// 	return this->contours.size();
// }
// size_t ContourPipe::findLargest(const cv::Mat& binary_frame) {
// 	this->largest = 0.f;
// 	this->target = -1;

// 	this->findContours(binary_frame);
// 	for(size_t i = 0; i < this->contours.size(); i++) {
// 		this->area = cv::contourArea(this->contours[i]);
// 		if(this->area > this->largest) {
// 			this->largest = this->area;
// 			this->target = i;
// 		}
// 	}
// 	return this->target;
// }
// size_t ContourPipe::findLargestGT(const cv::Mat& binary_frame, double area_gt) {
// 	this->largest = 0.f;
// 	this->target = -1;

// 	this->findContours(binary_frame);
// 	for(size_t i = 0; i < this->contours.size(); i++) {
// 		this->area = cv::contourArea(this->contours[i]);
// 		if(this->area > area_gt && this->area > this->largest) {
// 			this->largest = this->area;
// 			this->target = i;
// 		}
// 	}
// 	return this->target;
// }
// size_t ContourPipe::getTarget() const {
// 	return this->target;
// }