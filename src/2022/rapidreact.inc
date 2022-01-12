#include "rapidreact.h"

#include <algorithm>

template<typename num_t> 
void UpperHub::sort(const std::vector<cv::Point_<num_t> >& contour) {
	static_assert(std::is_arithmetic<num_t>::value, "Number type must be arithmetic");
	this->points.clear();
	size_t limit = contour.size() < 5 ? contour.size() : 5;
	for(size_t i = 0; i < limit; i++) {
		this->points.push_back(contour[i]);
	}
	std::sort(
		this->points.begin(),
		this->points.end(),
		[this](const cv::Point2f& a, const cv::Point2f& b) { return a.x < b.x; }
	);
	for(size_t i = 0; i < this->points.size()-1; i++) {
		this->comparisons[i] = this->points[i+1].x - this->points[i].x;
	}
	if(this->points.size()%2 == 0) {
		for(size_t i = 1; i < this->world.size(); i+=2) {
			this->world_match[i/2] = this->world[i];
		}
	} else {

	}

}

template<VThreshold::LED color>
StripFinder<color>::StripFinder(VisionServer& server) :
	VPipeline(server, "Upper-Hub vision strip finder"), WeightedSubtraction<color>(server, this->table)
{
	this->table->PutBoolean("Show Thresholded", false);
}

template<VThreshold::LED color>
void StripFinder<color>::process(cv::Mat& io_frame, int8_t mode) {

	this->findContours(this->threshold(io_frame));
	if(this->table->GetBoolean("Show Thresholded", false)) {
		this->fromBinary(io_frame);
	}

	this->filtered.clear();
	for(size_t i = 0; i < this->contours.size(); i++) {
		cv::convexHull(this->contours[i], this->point_buffer);
		cv::approxPolyDP(this->point_buffer, this->point_buffer, 0.1*cv::arcLength(this->contours[i], false), true);
		if(true/*this->point_buffer.size() == 4/* || this->point_buffer.size() == 3*/) {
			this->filtered.emplace_back(std::move(this->point_buffer));
			this->point_buffer.clear();
		}
	}
	this->point_buffer.clear();
	for(size_t i = 0; i < this->filtered.size(); i++) {
		rescale(this->filtered[i], this->getScale());
		this->point_buffer.emplace_back(std::move(::findCenter(this->filtered[i])));
		cv::circle(io_frame, this->point_buffer[i], 1, cv::Scalar(255, 255, 0), 2);
		// if(this->filtered[i].size() > this->highest) {
		// 	this->highest = this->filtered[i].size();
		// }
		cv::putText(
			io_frame, std::to_string(this->filtered[i].size()), 
			this->point_buffer[i], 
			cv::FONT_HERSHEY_DUPLEX, 0.7, cv::Scalar(255, 0, 0), 1, cv::LINE_AA
		);
	}
	// cv::putText(
	// 	io_frame, std::to_string(this->highest), 
	// 	cv::Point(200, 200), 
	// 	cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(255, 0, 0), 1, cv::LINE_AA
	// );

}