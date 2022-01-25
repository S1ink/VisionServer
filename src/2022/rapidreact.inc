#include "rapidreact.h"

#include <algorithm>

template<typename num_t>
void UpperHub::sort(std::vector<cv::Rect_<num_t> >& contour) {
	static_assert(std::is_arithmetic<num_t>::value, "Number type must be arithmetic");
	this->points.clear();
	std::sort(
		contour.begin(),
		contour.end(),
		[](const cv::Rect_<num_t>& a, const cv::Rect_<num_t>& b) { 
			return a.x < b.x; 
		}
	);
	num_t sample_width = 0;
	for(size_t i = 0; i < contour.size(); i++) {
		this->points.emplace_back(std::move(::findCenter<float, num_t>(contour[i])));
		if(contour[i].width > sample_width) {
			sample_width = contour[i].width;
		}
	}
	sample_width *= 2;
	size_t world_idx = 0;
	for(size_t i = 1; i <= this->points.size() && world_idx < this->world.size(); i++) {
		this->world_match[i-1] = this->world[world_idx];
		if(i < this->points.size()) {
			world_idx += (size_t)((std::abs(this->points[i].x - this->points[i-1].x)/sample_width)+0.5f);	// find distance, divide by width, round
		}
	}
}
template<typename num_t> 
void UpperHub::sort(const std::vector<cv::Point_<num_t> >& contour) {
	static_assert(std::is_arithmetic<num_t>::value, "Number type must be arithmetic");
	this->points.clear();
	size_t limit = contour.size() < 6 ? contour.size() : 6;
	for(size_t i = 0; i < limit; i++) {
		this->points.push_back(contour[i]);
	}
	std::sort(	// sort left to right
		this->points.begin(),
		this->points.end(),
		[](const cv::Point2f& a, const cv::Point2f& b) { return a.x < b.x; }
	);
	this->matchWorld();
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

	size_t reinserted = 0;
	this->in_range.clear();
	for(size_t i = 0; i < this->contours.size(); i++) {
		::rescale(this->contours[i], this->getScale());
	}
	for(size_t i = 0; i < this->contours.size(); i++) {
		if(i < reinserted) { i = reinserted; }

		this->rect_buffer = cv::boundingRect(this->contours[i]);
		if(::inRange(((float)this->rect_buffer.width/this->rect_buffer.height), 1.75f, 2.75f)) {

			this->in_range.push_back(this->rect_buffer);
			this->range = cv::Size(this->rect_buffer.width*7, this->rect_buffer.height*4);

			std::iter_swap(this->contours.begin() + reinserted, this->contours.begin() + i);	//make sure we don't check this value again in either loop
			reinserted++;
			
				cv::rectangle(io_frame, this->rect_buffer, cv::Scalar(0, 255, 255));	//debug
				//cv::rectangle(io_frame, this->rect_buffer.tl() - this->range, this->rect_buffer.tl() + this->range, cv::Scalar(0, 100, 255));	//debug
				//cv::putText(io_frame, std::to_string(i), this->rect_buffer.tl(), cv::FONT_HERSHEY_DUPLEX, 0.75, cv::Scalar(255, 0, 255));

			for(size_t n = reinserted; n < this->contours.size(); n++) {
				
				this->rect_buffer = cv::boundingRect(this->contours[n]);
				if(::distance(this->rect_buffer.tl(), this->in_range[0].tl()) <= this->range) {
					
					float ratio = (float)this->rect_buffer.width/this->rect_buffer.height;
					if(::inRange(ratio, 1.75f, 2.75f) && this->rect_buffer.area() > this->in_range[0].area()) {
						
						this->in_range.insert(this->in_range.begin(), this->rect_buffer);
						this->range = cv::Size(this->rect_buffer.width * 6, this->rect_buffer.height * 3);

						std::iter_swap(this->contours.begin() + reinserted, this->contours.begin() + n);	//make sure we don't check this value again becuase it passes the first test by default if in this block
						reinserted++;
						
							cv::rectangle(io_frame, this->rect_buffer, cv::Scalar(0, 255, 0));	//debug
							//cv::rectangle(io_frame, this->rect_buffer.tl()-this->range, this->rect_buffer.tl()+this->range, cv::Scalar(0, 200, 255));	//debug
							//cv::putText(io_frame, std::to_string(i), this->rect_buffer.tl(), cv::FONT_HERSHEY_DUPLEX, 0.75, cv::Scalar(255, 0, 255));
					
					} else if(::inRange(ratio, 0.75f, 3.f) && ::inRange((float)this->rect_buffer.area(), 0.5f*this->in_range[0].area(), 2.f*this->in_range[0].area())) {
						this->in_range.push_back(this->rect_buffer);	//this doesn't qualify for reinsertion because the requirements are less strict
							
							cv::rectangle(io_frame, this->rect_buffer, cv::Scalar(255, 255, 0));	//debug
					}
				}
			}
			if(::inRange(this->in_range.size(), 4U, 6U)) {
				this->target.resetPoints();
				for(size_t n = 0; n < this->in_range.size(); n++) {
					this->target.add(::findCenter<float, int>(this->in_range[n]));
				}
				this->target.sort();
				for(size_t i = 0; i < this->target.size(); i++) {
					cv::putText(io_frame, std::to_string(i), this->target.getPoints()[i], cv::FONT_HERSHEY_DUPLEX, 0.75, cv::Scalar(255, 0, 255));
					cv::circle(io_frame, this->target.getPoints()[i], 0, cv::Scalar(255, 255, 0), 2);
				}
				this->target.solvePerspective(this->tvec, this->rvec, this->getCameraMatrix(), this->getCameraDistortion(), cv::SOLVEPNP_IPPE);
				this->updateTarget(this->target.getName());

				cv::projectPoints(this->pose3D, this->rvec, this->tvec, this->getCameraMatrix(), this->getCameraDistortion(), this->pose2D);
				cv::line(io_frame, this->pose2D[0], this->pose2D[1], cv::Scalar(0, 255, 0));

				break;
			} else {
				this->in_range.clear();
			}
		} else {
				cv::rectangle(io_frame, this->rect_buffer, cv::Scalar(0, 0, 255));	//debug
		}
	}

	// this->target_idx = -1;
	// this->area_largest = 0.f;
	// for(size_t i = 0; i < this->contours.size(); i++) {
	// 	this->extent = cv::boundingRect(this->contours[i]);
	// 	if(
	// 		this->extent.width >= 1.75*this->extent.height && this->extent.width <= 2.75*this->extent.height &&
	// 		this->extent.area() > this->area_largest
	// 	) {
	// 		this->target_idx = i;
	// 		this->area_largest = this->extent.area();
	// 	} 
	// }
	// if(this->target_idx > -1) {
	// 	this->extent = cv::boundingRect(this->contours[this->target_idx]);
	// 		cv::rectangle(io_frame, this->extent, cv::Scalar(0, 255, 0));
	// 	this->point_buffer.clear();
	// 	this->point_buffer.emplace_back(std::move(::findCenter<int>({this->extent.tl(), this->extent.br()})));
	// 	cv::Size2i range = cv::Size2i(this->extent.width*7, this->extent.height*4);
	// 		cv::rectangle(io_frame, this->point_buffer[0] - range, this->point_buffer[0] + range, cv::Scalar(0, 255, 255));
	// 	for(size_t i = 0; i < this->contours.size(); i++) {
	// 		if(i != this->target_idx) {
	// 			this->extent = cv::boundingRect(this->contours[i]);
	// 			if(std::abs(this->point_buffer[0].x-this->extent.x) <= range.width && std::abs(this->point_buffer[0].y-this->extent.y) <= range.height) {
	// 					cv::rectangle(io_frame, this->extent, cv::Scalar(255, 255, 0));
	// 				this->point_buffer.emplace_back(std::move(::findCenter<int>({this->extent.tl(), this->extent.br()})));
	// 			}	 else {
	// 					cv::rectangle(io_frame, this->extent, cv::Scalar(0, 0, 255));
	// 				}
	// 		}
	// 	}
	// 	if(this->target.compatible(this->point_buffer)) {
	// 		::rescale(this->point_buffer, this->getScale());
	// 		this->target.sort(this->point_buffer);
	// 		this->target.solvePerspective(this->tvec, this->rvec, this->getCameraMatrix(), this->getCameraDistortion());

	// 		this->updateTarget(this->target.getName());

	// 			for(size_t i = 0; i < this->point_buffer.size(); i++) {
	// 				cv::circle(io_frame, this->point_buffer[i], 0, cv::Scalar(255, 255, 0), 2);
	// 			}
	// 	}
	// } else {
	// 	cv::putText(
	// 		io_frame, "!", 
	// 		cv::Point(io_frame.size().width*0.9, io_frame.size().height*0.9), 
	// 		cv::FONT_HERSHEY_DUPLEX, 2, cv::Scalar(0, 0, 255), 1, cv::LINE_AA
	// 	);
	// }



	// this->filtered.clear();
	// for(size_t i = 0; i < this->contours.size(); i++) {
	// 	cv::convexHull(this->contours[i], this->point_buffer);
	// 	cv::approxPolyDP(this->point_buffer, this->point_buffer, 0.1*cv::arcLength(this->contours[i], false), true);
	// 	if(this->point_buffer.size() == 4) {
	// 		this->candidates.emplace_back(this->point_buffer);
	// 		this->point_buffer.clear();
	// 	}
	// }
	// if(this->candidates.size() > 0) {
	// 	int upper = this->candidates[0].center + this->candidates[0].extent.width*6;
	// 	int lower = this->candidates[0].center - this->candidates[0].extent.width*6;
	// 	for(size_t i = 0; i < this->contours.size(); i++) {
	// 		this->candidates_buffer.load(this->contours[i]);
	// 		if()
	// 	}
	// }
	
	// this->point_buffer.clear();
	// for(size_t i = 0; i < this->filtered.size(); i++) {
	// 	rescale(this->filtered[i], this->getScale());
	// 	this->point_buffer.emplace_back(std::move(::findCenter(this->filtered[i])));
	// 	cv::circle(io_frame, this->point_buffer[i], 1, cv::Scalar(255, 255, 0), 2);
	// 	// if(this->filtered[i].size() > this->highest) {
	// 	// 	this->highest = this->filtered[i].size();
	// 	// }
	// 	cv::putText(
	// 		io_frame, std::to_string(this->filtered[i].size()), 
	// 		this->point_buffer[i], 
	// 		cv::FONT_HERSHEY_DUPLEX, 0.7, cv::Scalar(255, 0, 0), 1, cv::LINE_AA
	// 	);
	// }
	// cv::putText(
	// 	io_frame, std::to_string(this->highest), 
	// 	cv::Point(200, 200), 
	// 	cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(255, 0, 0), 1, cv::LINE_AA
	// );

	// std::sort(
	// 	this->contours.begin(),
	// 	this->contours.end(),
	// 	[this](const std::vector<cv::Point>& a, const std::vector<cv::Point>& b) { 
	// 		return ::findCenter(a).x < ::findCenter(b).x; 
	// 	}
	// );

	// cv::Point center;
	// for(size_t i = 0; i < this->contours.size(); i++) {
	// 	::rescale(this->contours[i], this->getScale());
	// 	for(size_t ii = 0; ii < this->contours[i].size(); ii++) {
	// 		cv::circle(io_frame, this->contours[i][ii], 0, cv::Scalar(0, 0, 255), 2);
	// 	}
	// 	center = ::findCenter(this->contours[i]);
	// 	cv::line(io_frame, center, cv::Point(i*70+25, io_frame.size().height*0.80), cv::Scalar(255, 0, 255));
	// 	cv::Rect size = cv::boundingRect(this->contours[i]);
	// 	cv::putText(
	// 		io_frame, std::to_string(size.width) + "x" + std::to_string(size.height),
	// 		cv::Point(i*70+10, io_frame.size().height*0.85), cv::FONT_HERSHEY_DUPLEX, 0.7, cv::Scalar(255, 0, 0), 1, cv::LINE_AA
	// 	);
	// 	cv::putText(
	// 		io_frame, "Y:" + std::to_string(size.y),
	// 		cv::Point(i*70+10, io_frame.size().height*0.9), cv::FONT_HERSHEY_DUPLEX, 0.7, cv::Scalar(255, 0, 0), 1, cv::LINE_AA
	// 	);
	// 	cv::putText(
	// 		io_frame, "V:" + std::to_string(io_frame.at<uchar>(center)),
	// 		cv::Point(i*70+10, io_frame.size().height*0.95), cv::FONT_HERSHEY_DUPLEX, 0.7, cv::Scalar(255, 0, 0), 1, cv::LINE_AA
	// 	);
	//}

}