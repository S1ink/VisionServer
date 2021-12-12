#include "processing.h"

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
			return -atan2(buff1.x, -buff1.y) < -atan2(buff2.x, -buff2.y);
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