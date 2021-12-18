//#include "processing.h"

#include "pipelines.h"
#include "vision.h"

template<size_t corners>
Target<corners>::Target(const std::array<cv::Point3f, corners>& world_pts) : world(world_pts) {}

template<size_t corners>
size_t Target<corners>::getSize() {
	return corners;
}

template<size_t corners>
void Target<corners>::sort(const std::vector<cv::Point2i>& contour) {
	cv::Point2f center = findCenter<float>(contour), buff1, buff2;
	size_t limit = corners > contour.size() ? contour.size() : corners;
	for(size_t i = 0; i < limit; i++) {
		this->points[i] = contour[i];
	}
	std::sort(
		this->points.begin(), 
		this->points.end(), 
		[this, center, &buff1, &buff2](const cv::Point2f& a, const cv::Point2f& b) {
			buff1 = a - center;
			buff2 = b - center;
			return -atan2(buff1.x, buff1.y) < -atan2(buff2.x, buff2.y);	// note that x and y are switched -> clockwise
		}
	);
}
template<size_t corners>
void Target<corners>::rescale(double scale) {
	for(size_t i = 0; i < corners; i++) {
		this->points[i] *= scale;
	}
}
template<size_t corners>
std::array<cv::Point2f, corners> Target<corners>::getRescaled(double scale) {
	std::array<cv::Point2f, corners> ret;
	for(size_t i = 0; i < corners; i++) {
		ret[i] = this->points[i] * scale;
	}
	return ret;
}



template<VThreshold::LED color>
const std::array<std::array<uint8_t, 2>, 3> WSThreshold<color>::weighted_array = {
	std::array<uint8_t, 2>{(uint8_t)LED::GREEN, (uint8_t)LED::RED},
	std::array<uint8_t, 2>{(uint8_t)LED::BLUE, (uint8_t)LED::RED},
	std::array<uint8_t, 2>{(uint8_t)LED::BLUE, (uint8_t)LED::GREEN},
};
template<VThreshold::LED color>
WSThreshold<color>::WSThreshold(cv::Size frame_sz, std::shared_ptr<nt::NetworkTable> table) {
	std::cout << "Exhibit B\n";
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