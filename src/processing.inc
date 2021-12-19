#include "processing.h"

#include "pipelines.h"
#include "vision.h"

template<VThreshold::LED color>
const std::array<std::array<uint8_t, 2>, 3> WSThreshold<color>::weighted_array = {
	std::array<uint8_t, 2>{(uint8_t)LED::GREEN, (uint8_t)LED::RED},
	std::array<uint8_t, 2>{(uint8_t)LED::BLUE, (uint8_t)LED::RED},
	std::array<uint8_t, 2>{(uint8_t)LED::BLUE, (uint8_t)LED::GREEN},
};
template<VThreshold::LED color>
WSThreshold<color>::WSThreshold(cv::Size frame_sz, std::shared_ptr<nt::NetworkTable> table) {
	addNetTableVar(this->weight, "Weight", table);
	addNetTableVar(this->thresh, "Threshold", table);

	this->resizeBuffers(frame_sz);	// might not need
}
template<VThreshold::LED color>
void WSThreshold<color>::resizeBuffers(cv::Size size) {
	this->buffer = cv::Mat(size/this->scale, CV_8UC3);
	this->binary = cv::Mat(size/this->scale, CV_8UC1);
	for(size_t i = 0; i < this->channels.size(); i++) {
		channels[i] = cv::Mat(size/this->scale, CV_8UC1);
	}
}
template<VThreshold::LED color>
void WSThreshold<color>::threshold(const cv::Mat& io_frame) {
	if(io_frame.size() != this->buffer.size()*(int)this->scale) {	// optional
		this->resizeBuffers(io_frame.size());
	}

	cv::resize(io_frame, this->buffer, cv::Size(), 1.0/this->scale, 1.0/this->scale);
	cv::split(this->buffer, this->channels);
	cv::addWeighted(this->channels[weighted_array[(size_t)color][0]], this->weight, this->channels[weighted_array[(size_t)color][1]], this->weight, 0.0, this->binary);
	cv::subtract(this->channels[(size_t)color], this->binary, this->binary);
	memcpy_threshold_binary_asm(this->binary.data, this->binary.data, this->binary.size().area(), this->thresh);
}
template<VThreshold::LED color>
size_t WSThreshold<color>::getScale() const {
	return this->scale;
}
template<VThreshold::LED color>
uint8_t WSThreshold<color>::getThresh() const {
	return this->thresh;
}
template<VThreshold::LED color>
void WSThreshold<color>::setThresh(uint8_t t) {
	this->thresh = t;
}
template<VThreshold::LED color>
double WSThreshold<color>::getWeight() const {
	return this->weight;
}
template<VThreshold::LED color>
void WSThreshold<color>::setWeight(double w) {
	this->weight = w;
}