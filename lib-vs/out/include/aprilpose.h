#pragma once

#include <vector>
#include <string>

#include <opencv2/aruco.hpp>

#include "visionserver2.h"
#include "vision.h"


constexpr static inline cv::aruco::PREDEFINED_DICTIONARY_NAME
	FRC_DICT = cv::aruco::DICT_APRILTAG_16h5,
	FRC_UNOFF_DICT = cv::aruco::DICT_APRILTAG_36h11;
constexpr static inline char const
	* FRC_DICT_NAME = "tag16h5",
	* FRC_UNOFF_DICT_NAME = "tag36h11";

template<class derived_t = void>
class AprilPose_ : public vs2::VPipeline<AprilPose_<derived_t> > {
	using This_t = AprilPose_<derived_t>;
public:
	inline AprilPose_(
		const cv::Ptr<cv::aruco::Board> f,
		cv::Ptr<cv::aruco::DetectorParameters> p = cv::aruco::DetectorParameters::create()
	) : vs2::VPipeline<This_t>("AprilTag Pose Estimator"), params{p}, markers{f->dictionary}, field{f}
	{
		this->getTable()->PutNumber("Scaling", this->scale);
		this->getTable()->PutNumber("Decimate", this->params->aprilTagQuadDecimate);
	}

	inline virtual void process(cv::Mat& io_frame) override {
		this->_detect(io_frame);
		this->_estimate(io_frame);
		this->_draw(io_frame);
#ifdef APRILPOSE_DEBUG
		this->_profile(io_frame);
#endif
	}

protected:
	inline AprilPose_(
		const char* n, const cv::Ptr<cv::aruco::Board> f,
		cv::Ptr<cv::aruco::DetectorParameters> p = cv::aruco::DetectorParameters::create()
	) : vs2::VPipeline<This_t>(n), params{p}, markers{f->dictionary}, field{f} {
			this->getTable()->PutNumber("Scaling", this->scale);
			this->getTable()->PutNumber("Decimate", this->params->aprilTagQuadDecimate);
		}
	inline AprilPose_(
		const std::string& n, const cv::Ptr<cv::aruco::Board> f,
		cv::Ptr<cv::aruco::DetectorParameters> p = cv::aruco::DetectorParameters::create()
	) : vs2::VPipeline<This_t>(n), params{p}, markers{f->dictionary}, field{f} {
			this->getTable()->PutNumber("Scaling", this->scale);
			this->getTable()->PutNumber("Decimate", this->params->aprilTagQuadDecimate);
		}
	inline AprilPose_(
		std::string&& n, const cv::Ptr<cv::aruco::Board> f,
		cv::Ptr<cv::aruco::DetectorParameters> p = cv::aruco::DetectorParameters::create()
	) : vs2::VPipeline<This_t>(n), params{p}, markers{f->dictionary}, field{f} {
			this->getTable()->PutNumber("Scaling", this->scale);
			this->getTable()->PutNumber("Decimate", this->params->aprilTagQuadDecimate);
		}

	void _detect(cv::Mat& io_frame);
	void _estimate(cv::Mat& io_frame);
	void _draw(cv::Mat& io_frame);
	
	cv::Ptr<cv::aruco::DetectorParameters> params;
	cv::Ptr<cv::aruco::Dictionary> markers;
	cv::Ptr<cv::aruco::Board> field;

	std::vector<std::vector<cv::Point2f> > corners;
	std::vector<int32_t> ids;
	std::array<float, 3> tvec{0.f}, rvec{0.f};
	
	cv::Mat buffer;
	size_t scale{1};

#ifdef APRILPOSE_DEBUG
	void _profile(cv::Mat& io_frame);

	std::array<float, 6> profiling{0.f};
#endif


};
typedef AprilPose_<>	AprilPose;





#ifdef APRILPOSE_DEBUG
#include <chrono>
using hrc = std::chrono::high_resolution_clock;
#endif

template<class derived>
void AprilPose_<derived>::_detect(cv::Mat&io_frame) {
#ifdef APRILPOSE_DEBUG
	hrc::time_point beg, end;
	beg = hrc::now();
#endif
	this->scale = this->getTable()->GetNumber("Scaling", 1.0);
	this->params->aprilTagQuadDecimate = this->getTable()->GetNumber("Decimate", this->params->aprilTagQuadDecimate);
	this->corners.clear();
	this->ids.clear();
	cv::Size2i fsz = io_frame.size() / this->scale;
	if(this->buffer.size() != fsz) {
		this->buffer = cv::Mat(fsz, CV_8UC3);
	}
#ifdef APRILPOSE_DEBUG
	end = hrc::now();
	this->profiling[0] = (end - beg).count() / 1e6;
	beg = hrc::now();
#endif
	cv::resize(io_frame, this->buffer, fsz);
#ifdef APRILPOSE_DEBUG
	end = HRC::now();
	this->profiling[1] = (end - beg).count() / 1e6;
	beg = HRC::now();
#endif
	cv::aruco::detectMarkers(
		this->buffer, this->markers,
		this->corners, this->ids,
		this->params
	);
#ifdef APRILPOSE_DEBUG
	end = HRC::now();
	this->profiling[2] = (end - beg).count() / 1e6;
	beg = HRC::now();
#endif
	if(this->scale != 1.0) {
		rescale(this->corners, this->scale);
	}
#ifdef APRILPOSE_DEBUG
	end = HRC::now();
	this->profiling[3] = (end - beg).count() / 1e6;
#endif
}

template<class derived>
void AprilPose_<derived>::_estimate(cv::Mat& io_frame) {
#ifdef APRILPOSE_DEBUG
	hrc::time_point beg, end;
	beg = hrc::now();
#endif
	cv::aruco::estimatePoseBoard(
		this->corners, this->ids, this->field,
		this->getSrcMatrix(), this->getSrcDistort(),
		this->rvec, this->tvec);
#ifdef APRILPOSE_DEBUG
	end = HRC::now();
	this->profiling[4] = (end - beg).count() / 1e6;
#endif
}

template<class derived>
void AprilPose_<derived>::_draw(cv::Mat& io_frame) {
#ifdef APRILPOSE_DEBUG
	hrc::time_point beg, end;
	beg = hrc::now();
#endif
	//cv::drawFrameAxes(io_frame, this->getSrcMatrix(), this->getSrcDistort(), this->rvec, this->tvec, 100.f);
	cv::aruco::drawDetectedMarkers(io_frame, this->corners, this->ids);
#ifdef APRILPOSE_DEBUG
	end = HRC::now();
	this->profiling[5] = (end - beg).count() / 1e6;
#endif
}

#ifdef APRILPOSE_DEBUG
template<class derived>
void AprilPose_<derived>::_profile(cv::Mat& io_frame) {
	cv::putText(
		io_frame, "P_init(ms): " + std::to_string(this->profiling[0]),
		{5, 240}, cv::FONT_HERSHEY_DUPLEX, 0.5, {255, 100, 0}
	);
	cv::putText(
		io_frame, "P_resize(ms): " + std::to_string(this->profiling[1]),
		{5, 260}, cv::FONT_HERSHEY_DUPLEX, 0.5, {255, 100, 0}
	);
	cv::putText(
		io_frame, "P_detect(ms): " + std::to_string(this->profiling[2]),
		{5, 280}, cv::FONT_HERSHEY_DUPLEX, 0.5, {255, 100, 0}
	);
	cv::putText(
		io_frame, "P_rescale(ms): " + std::to_string(this->profiling[3]),
		{5, 300}, cv::FONT_HERSHEY_DUPLEX, 0.5, {255, 100, 0}
	);
	cv::putText(
		io_frame, "P_estimate(ms): " + std::to_string(this->profiling[4]),
		{5, 320}, cv::FONT_HERSHEY_DUPLEX, 0.5, {255, 100, 0}
	);
	cv::putText(
		io_frame, "P_draw(ms): " + std::to_string(this->profiling[5]),
		{5, 340}, cv::FONT_HERSHEY_DUPLEX, 0.5, {255, 100, 0}
	);
}
#endif
