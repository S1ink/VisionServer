#include "rapidreact2.h"

#include "../core/vision.h"
#include "../core/mem.h"


UHPipeline::UHPipeline(vs2::BGR c) :
	vs2::VPipeline<UHPipeline>("Upper Hub Pipeline "), color(c)
{
	this->getTable()->PutBoolean("Output Threshold", false);
}
void UHPipeline::process(cv::Mat& io_frame) {

	if(io_frame.size() != this->buffer.size()*(int)this->scale) {	// resize buffers if change in dimensions
		this->buffer = cv::Mat(io_frame.size()/this->scale, CV_8UC3);
		this->binary = cv::Mat(io_frame.size()/this->scale, CV_8UC3);
		for(size_t i = 0; i < this->channels.size(); i++) {
			this->channels.at(i) = cv::Mat(io_frame.size()/this->scale, CV_8UC1);
		}
	}

	cv::resize(io_frame, this->buffer, {}, 1.0/this->scale, 1.0/this->scale);	// downscale image
	cv::split(this->buffer, this->channels);									// split into channels
	cv::addWeighted(															// add weights to out-colors
		this->channels[vs2::weights_map[~this->color][0]], this->alpha,
		this->channels[vs2::weights_map[~this->color][1]], this->beta,
		this->gamma, this->binary
	);
	cv::subtract(this->channels[~this->color], this->binary, this->binary);
	memcpy_threshold_binary_asm(this->binary.data, this->binary.data, this->binary.size().area(), this->thresh);

	this->contours.clear();
	cv::findContours(this->binary, this->contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	if(this->getTable()->GetBoolean("Output Threshold", false)) {
		cv::cvtColor(this->binary, this->buffer, cv::COLOR_GRAY2BGR, 3);
		cv::resize(this->buffer, io_frame, {}, this->scale, this->scale, cv::INTER_NEAREST);
	}

	size_t reinserted = 0;
	this->in_range.clear();
	for(size_t i = 0; i < this->contours.size(); i++) {
		::rescale(this->contours.at(i), this->scale);
	}
	for(size_t i = 0; i < this->contours.size(); i++) {
		if(i < reinserted) {
			i = reinserted;
		}
		this->rect_buff = cv::boundingRect(this->contours.at(i));
		if(::inRange(((float)this->rect_buff.width / this->rect_buff.height), 1.75f, 2.75f)) {
			
			this->in_range.push_back(this->rect_buff);
			this->range_buff = cv::Size(this->rect_buff.width * 7, this->rect_buff.height * 4);
			
			std::iter_swap(this->contours.begin() + reinserted, this->contours.begin() + i);
			reinserted++;

				cv::rectangle(io_frame, this->rect_buff, {0, 255, 255});	// debug

			for(size_t n = reinserted; n < this->contours.size(); n++) {

				this->rect_buff = cv::boundingRect(this->contours.at(n));
				if(::distance(this->rect_buff.tl(), this->in_range[0].tl()) <= this->range_buff) {

					float ratio = (float)this->rect_buff.width / this->rect_buff.height;
					if(::inRange(ratio, 1.75f, 2.75f) && this->rect_buff.area() > this->in_range[0].area()) {

						this->in_range.insert(this->in_range.begin(), this->rect_buff);
						this->range_buff = cv::Size(this->rect_buff.width * 6, this->rect_buff.height * 3);

						std::iter_swap(this->contours.begin() + reinserted, this->contours.begin() + n);
						reinserted++;

							cv::rectangle(io_frame, this->rect_buff, {0, 255, 0});	// debug

					} else if(
					::inRange(ratio, 0.75f, 3.f) &&
					::inRange((float)this->rect_buff.area(), 0.5f * this->in_range[0].area(), 2.f * this->in_range[0].area())
					) {
						this->in_range.push_back(this->rect_buff);

							cv::rectangle(io_frame, this->rect_buff, {255, 255, 0});	// debug

					}
				}
			}
			if(::inRange(this->in_range.size(), UpperHub::min, UpperHub::max)) {

				this->points_buff.clear();
				for(size_t m = 0; m < this->in_range.size(); m++) {
					this->points_buff.emplace_back(::findCenter<float, int>(this->in_range[m]));
				}
				std::sort(
					this->points_buff.begin(),
					this->points_buff.end(),
					[](const cv::Point2f& a, const cv::Point2f& b){ return a.x < b.x; }
				);
				this->world_buff.clear();
				this->world_buff.insert(
					this->world_buff.begin(),
					UpperHub::world_coords.begin(),
					UpperHub::world_coords.begin() + this->points_buff.size()
				);
				cv::solvePnPGeneric(
					this->world_buff, this->points_buff,
					this->getSrcMatrix(), this->getSrcDistort(),
					this->rvecs, this->tvecs,
					false, cv::SOLVEPNP_IPPE
				);
				int best =
					(this->tvecs[0].at<double>({1, 0}) + rvecs[0].at<double>({0, 0}))
					> (this->tvecs[1].at<double>({1, 0}) + rvecs[1].at<double>({0, 0}))
					? 0 : 1;
				this->tvec = this->tvecs[best];
				this->rvec = this->rvecs[best];
				cv::solvePnPRefineLM(
					this->world_buff, this->points_buff,
					this->getSrcMatrix(), this->getSrcDistort(),
					this->rvec, this->tvec
				);
				this->target.setPos(
					this->tvec[0][0],
					this->tvec[1][0],
					this->tvec[2][0]
				);
				this->target.setDist(
					sqrt(pow(this->tvec[0][0], 2) + pow(this->tvec[1][0], 2) + pow(this->tvec[2][0], 2))
				);
				this->target.setAngle(
					0.0,
					acos(this->tvec[2][0] / sqrt(pow(this->tvec[0][0], 2) + pow(this->tvec[2][0], 2)))
						* 180 / CV_PI * sgn(this->tvec[0][0])
				);

				// draw 3d debug

				break;

			} else {
				this->in_range.clear();
			}
		} else {

				cv::rectangle(io_frame, this->rect_buff, {0, 0, 255});	// debug

		}
	}

}