#include "weightedsubtraction.h"

#include "vision.h"
#include "mem.h"


namespace vs1 {

template<VThreshold::LED color>
const std::array<std::array<uint8_t, 2>, 3> WeightedSubtraction<color>::weighted_array = {
	std::array<uint8_t, 2>{(uint8_t)LED::GREEN, (uint8_t)LED::RED},
	std::array<uint8_t, 2>{(uint8_t)LED::BLUE, (uint8_t)LED::RED},
	std::array<uint8_t, 2>{(uint8_t)LED::BLUE, (uint8_t)LED::GREEN},
};
template<VThreshold::LED color>
WeightedSubtraction<color>::WeightedSubtraction(VisionServer& server, std::shared_ptr<nt::NetworkTable> table) :
	VThreshold(server)
{
	//addNetTableVar(this->weight, "Weight", table);
	addNetTableVar(this->thresh, "Threshold", table);
	addNetTableVar(this->scale, "Scaling", table);

	this->resizeBuffers(server.getCurrentResolution());
}
template<VThreshold::LED color>
WeightedSubtraction<color>::WeightedSubtraction(VisionServer& server) :
	VThreshold(server)
{
	this->resizeBuffers(server.getCurrentResolution());
}
template<VThreshold::LED color>
void WeightedSubtraction<color>::resizeBuffers(cv::Size size) {
	this->buffer = cv::Mat(size/this->scale, CV_8UC3);
	this->binary = cv::Mat(size/this->scale, CV_8UC1);
	for(size_t i = 0; i < this->channels.size(); i++) {
		channels[i] = cv::Mat(size/this->scale, CV_8UC1);
	}
}
template<VThreshold::LED color>
const cv::Mat& WeightedSubtraction<color>::threshold(const cv::Mat& io_frame) {
	if(io_frame.size() != this->buffer.size()*(int)this->scale) {
		this->resizeBuffers(io_frame.size());
	}

	cv::resize(io_frame, this->buffer, cv::Size(), 1.0/this->scale, 1.0/this->scale);
	cv::split(this->buffer, this->channels);
	cv::addWeighted(this->channels[weighted_array[~color][0]], this->alpha, this->channels[weighted_array[~color][1]], this->beta, this->gamma, this->binary);
	cv::subtract(this->channels[~color], this->binary, this->binary);
	memcpy_threshold_binary_asm(this->binary.data, this->binary.data, this->binary.size().area(), this->thresh);

	return this->binary;
}
template<VThreshold::LED color>
void WeightedSubtraction<color>::fromBinary(cv::Mat& o_frame) const {
	cv::cvtColor(this->binary, this->buffer, cv::COLOR_GRAY2BGR, 3);
	cv::resize(this->buffer, o_frame, cv::Size(), this->scale, this->scale, cv::INTER_NEAREST);
}
template<VThreshold::LED color>
size_t WeightedSubtraction<color>::getScale() const {
	return this->scale;
}
template<VThreshold::LED color>
uint8_t WeightedSubtraction<color>::getThresh() const {
	return this->thresh;
}
template<VThreshold::LED color>
void WeightedSubtraction<color>::setThresh(uint8_t t) {
	this->thresh = t;
}
template<VThreshold::LED color>
double WeightedSubtraction<color>::getWeight() const {
	return this->weight;
}
template<VThreshold::LED color>
void WeightedSubtraction<color>::setWeight(double w) {
	this->weight = w;
}

}	// namespace vs1