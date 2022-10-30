#pragma once

#include <vector>
#include <string>

#include <opencv2/aruco.hpp>

#include "visionserver2.h"
#include "vision.h"


constexpr static inline cv::aruco::PREDEFINED_DICTIONARY_NAME
	FRC_DICT = cv::aruco::DICT_APRILTAG_36h11;

template<class derived = void>
class AprilPose_ : public vs2::VPipeline<AprilPose_<derived> > {
	using This_t = AprilPose_<derived>;
public:
	inline AprilPose_(
		const cv::Ptr<cv::aruco::Board> f,
		cv::Ptr<cv::aruco::DetectorParameters> p = cv::aruco::DetectorParameters::create()
	) : vs2::VPipeline<This_t>("AprilTag Pose Estimator "), field{f}, markers{f->dictionary}, params{p}
	{
		this->getTable()->PutNumber("Scaling", this->scale);
	}

	virtual void process(cv::Mat& io_frame) override;

protected:	// update these
	inline AprilPose_(const char* n) : vs2::VPipeline<This_t>(n) {}
	inline AprilPose_(const std::string& n) : vs2::VPipeline<This_t>(n) {}
	inline AprilPose_(std::string&& n) : vs2::VPipeline<This_t>(std::move(n)) {}

	
	cv::Ptr<cv::aruco::Board> field;
	cv::Ptr<cv::aruco::Dictionary> markers;
	cv::Ptr<cv::aruco::DetectorParameters> params;

	std::vector<std::vector<cv::Point2f> > corners;
	std::vector<int32_t> ids;
	std::array<float, 3> tvec, rvec;
	
	cv::Mat buffer;
	size_t scale{2};


};
typedef AprilPose_<>	AprilPose;





#ifdef APRILPOSE_DEBUG
template<class derived>
void AprilPose_<derived>::process(cv::Mat& io_frame) {
	this->scale = this->getTable()->GetNumber("Scaling", 2.0);
	this->corners.clear();
	this->ids.clear();
	cv::Size2i fsz = io_frame.size() / this->scale;
	if(this->buffer.size() != fsz) {
		this->buffer = cv::Mat(fsz, CV_8UC3);
	}

	cv::resize(io_frame, this->buffer, fsz);
	cv::aruco::detectMarkers(
		this->buffer, this->markers,
		this->corners, this->ids,
		this->params
	);

	rescale(this->corners, this->scale);
	if(cv::aruco::estimatePoseBoard(
		this->corners, this->ids, this->field,
		this->getSrcMatrix(), this->getSrcDistort(),
		this->rvec, this->tvec
	)) {
		cv::drawFrameAxis(io_frame, this->getSrcMatrix(), this->getSrcDistort(), this->rvec, this->tvec, 100.f);
	}
	cv::aruco::drawDetectedMarkers(io_frame, this->corners, this->ids);
}
#else
#include <chrono>
#define HRC std::chrono::high_resolution_clock
template<class derived>
void AprilPose_<derived>::process(cv::Mat& io_frame) {
	this->scale = this->getTable()->GetNumber("Scaling", 2.0);
	HRC::time_point beg, end;
	beg = HRC::now();
	this->corners.clear();
	this->ids.clear();
	cv::Size2i fsz = io_frame.size() / this->scale;
	if(this->buffer.size() != fsz) {
		this->buffer = cv::Mat(fsz, CV_8UC3);
	}
	end = HRC::now();
	cv::putText(
		io_frame, "P_init(ms): " + std::to_string((end - beg).count() / 1e6),
		{5, 240}, cv::FONT_HERSHEY_DUPLEX, 0.5, {255, 100, 0}
	);
	beg = HRC::now();
	cv::resize(io_frame, this->buffer, fsz);
	end = HRC::now();
	cv::putText(
		io_frame, "P_resize(ms): " + std::to_string((end - beg).count() / 1e6),
		{5, 260}, cv::FONT_HERSHEY_DUPLEX, 0.5, {255, 100, 0}
	);
	beg = HRC::now();
	cv::aruco::detectMarkers(this->buffer, this->markers, this->corners, this->ids);
	end = HRC::now();
	cv::putText(
		io_frame, "P_detect(ms): " + std::to_string((end - beg).count() / 1e6),
		{5, 280}, cv::FONT_HERSHEY_DUPLEX, 0.5, {255, 100, 0}
	);
	if(this->corners.size() > 0) {
		beg = HRC::now();
		rescale(this->corners, this->scale);
		end = HRC::now();
		cv::putText(
			io_frame, "P_rescale(ms): " + std::to_string((end - beg).count() / 1e6),
			{5, 300}, cv::FONT_HERSHEY_DUPLEX, 0.5, {255, 100, 0}
		);
		beg = HRC::now();
		if(cv::aruco::estimatePoseBoard(
			this->corners, this->ids, this->field,
			this->getSrcMatrix(), this->getSrcDistort(),
			this->rvec, this->tvec
		)) {
			end = HRC::now();
			double t = (end-beg).count() / 1e6;
			cv::putText(
				io_frame, "P_estimate(ms): " + std::to_string(t),
				{5, 320}, cv::FONT_HERSHEY_DUPLEX, 0.5, {255, 100, 0}
			);
			if(t > 20.f) {
				std::cout << "OVERRUN: Estimate time of " << t << " ms" << std::endl;
			}
			beg = HRC::now();
			cv::drawFrameAxes(io_frame, this->getSrcMatrix(), this->getSrcDistort(), this->rvec, this->tvec, 100.f);
		} else {
			end = HRC::now();
			double t = (end-beg).count() / 1e6;
			cv::putText(
				io_frame, "P_estimate(ms): " + std::to_string(t),
				{5, 320}, cv::FONT_HERSHEY_DUPLEX, 0.5, {255, 100, 0}
			);
			if(t > 20.f) {
				std::cout << "OVERRUN: Estimate time of " << t << " ms" << std::endl;
			}
			beg = HRC::now();
		}
		cv::aruco::drawDetectedMarkers(io_frame, this->corners, this->ids);
		end = HRC::now();
		double t = (end-beg).count() / 1e6;
		cv::putText(
			io_frame, "P_draw(ms): " + std::to_string(t),
			{5, 340}, cv::FONT_HERSHEY_DUPLEX, 0.5, {255, 100, 0}
		);
		if(t > 20.f) {
			std::cout << "OVERRUN: Draw time of " << t << " ms" << std::endl;
		}
	}
}
#endif