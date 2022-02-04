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
	VPipeline(server, "Upper-Hub Pipeline"), WeightedSubtraction<color>(server, this->table)
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
				cv::line(io_frame, this->pose2D[1], this->pose2D[2], cv::Scalar(0, 255, 0));
				cv::line(io_frame, this->pose2D[1], this->pose2D[3], cv::Scalar(0, 255, 0));
				for(size_t r = 0; r < this->target.rmat.rows; r++) {
					for(size_t c = 0; c < this->target.rmat.cols; c++) {
						cv::putText(
							io_frame, 
							std::to_string(this->target.rmat[r][c]*180/M_PI),
							cv::Point(c*100, r*20 + 20),
							cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(255, 255, 0)
						);
					}
				}

				break;
			} else {
				this->in_range.clear();
			}
		} else {
				cv::rectangle(io_frame, this->rect_buffer, cv::Scalar(0, 0, 255));	//debug
		}
	}

}

template<VThreshold::LED primary, uint8_t alpha, uint8_t beta, uint8_t thresh_percent>
CargoFinder::BallFilter<primary, alpha, beta, thresh_percent>::BallFilter(VisionServer& server, CargoFinder& env) :
	WeightedSubtraction<primary>(server), env(&env)
{
	this->alpha = alpha/100.0;
	this->beta = beta/100.0;
	this->gamma = thresh_percent/100.0;
}

template<VThreshold::LED primary, uint8_t alpha, uint8_t beta, uint8_t thresh_percent>
void CargoFinder::BallFilter<primary, alpha, beta, thresh_percent>::threshold(cv::Mat& o_frame, const std::array<cv::Mat, 3>& channels) {
	cv::addWeighted(channels[this->weighted_array[~primary][0]], this->alpha, channels[this->weighted_array[~primary][1]], this->beta, 0.0, this->binary);
	cv::subtract(channels[~primary], this->binary, this->binary);
	cv::minMaxIdx(this->binary, nullptr, &this->max_val);
	memcpy_threshold_asm(this->binary.data, this->binary.data, this->binary.size().area(), this->max_val*this->gamma);

	if(this->env->table->GetBoolean("Show Threshold", false)) {
		this->fromBinary(this->buffer);
	}

	this->findContours(this->binary);

	this->filtered.clear();
	this->target_idx = -1;
	this->area_largest = 0.0;
	for(size_t i = 0; i < this->contours.size(); i++) {
		if(cv::contourArea(this->contours[i]) > this->binary.size().area()/500.0) {
			cv::minEnclosingCircle(this->contours[i], this->outline_buffer.center, this->outline_buffer.radius);
			cv::convexHull(this->contours[i], this->point_buffer);
			if(cv::contourArea(this->point_buffer)/(CV_PI*pow(this->outline_buffer.radius, 2)) > 0.8) {
				this->outline_buffer.color = (primary == VThreshold::LED::BLUE ? CargoColor::BLUE : (primary == VThreshold::LED::RED ? CargoColor::RED : CargoColor::NONE));
				this->outline_buffer.center *= (int)this->scale;
				this->outline_buffer.radius *= this->scale;
				this->filtered.emplace_back(this->outline_buffer);
				if(this->outline_buffer.radius > this->area_largest) {
					this->target_idx = this->filtered.size()-1;
					this->area_largest = this->outline_buffer.radius;
				}
				cv::circle(o_frame, this->outline_buffer.center, this->outline_buffer.radius, ::markup_map[~primary][0], 3);
			}
			if(this->env->table->GetBoolean("Show Contours", false)) {
				::rescale(this->contours[i], this->scale);
				cv::drawContours(o_frame, this->contours, i, ::markup_map[~primary][1]);
			}
		}
	}

}
template<VThreshold::LED primary, uint8_t alpha, uint8_t beta, uint8_t thresh_percent>
bool CargoFinder::BallFilter<primary, alpha, beta, thresh_percent>::launchThresholding(cv::Mat& o_frame, const std::array<cv::Mat, 3>& channels) { 
	if(!this->worker.joinable()) {
		this->worker = std::thread(thresholdWorker, this, std::ref(o_frame), std::cref(channels));
		return true;
	} 
	return false;
}
template<VThreshold::LED primary, uint8_t alpha, uint8_t beta, uint8_t thresh_percent>
bool CargoFinder::BallFilter<primary, alpha, beta, thresh_percent>::join() {
	if(this->worker.joinable()) {
		this->worker.join();
		return true;
	}
	return false;
}