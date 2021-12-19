#include "targets.h"

void TestingSquare::reorder(const std::vector<cv::Point2i>& contour) {
	this->center = findCenter<float>(contour);
	this->limit = this->size() > contour.size() ? contour.size() : this->size();
	for(size_t i = 0; i < limit; i++) {
		this->points[i] = contour[i];
	}
	std::sort(
		this->points.begin(), 
		this->points.end(), 
		[this](const cv::Point2f& a, const cv::Point2f& b) {
			this->a = a - this->center;
			this->b = b - this->center;
			return -atan2(this->a.x, this->a.y) < -atan2(this->b.x, this->b.y);
		}
	);
}