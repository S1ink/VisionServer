#pragma once

#include <vector>
#include <array>
#include <queue>
#include <string>
#include <type_traits>

#include <opencv2/opencv.hpp>

#include <networktables/NetworkTable.h>

#include "visionserver2.h"


namespace vs2 {

template<size_t points>
class Target {
public:
	Target() = delete;
	Target(const std::array<cv::Point3f, points>& world, const char* name);
	Target(const std::array<cv::Point3f, points>& world, const std::string& name);
	Target(const std::array<cv::Point3f, points>& world, std::string&& name);
	virtual ~Target();

	const std::array<cv::Point3f, points> world;
	static inline const size_t size{points};

	void solvePerspective(
		cv::Mat_<float>& tvec, cv::Mat_<float>& rvec,
		cv::InputArray camera_matrix, cv::InputArray camera_coeffs,
		int flags = 0, bool ext_guess = false
	);

protected:
	const std::string name;
	const std::shared_ptr<nt::NetworkTable> table;

	std::array<cv::Point3f, points> point_buff;


};
template<size_t points, class derived>
class UniqueTarget : Target<points> {
	typedef struct UniqueTarget<points, derived>	This_t;
public:
	UniqueTarget() = delete;
	inline UniqueTarget(const std::array<cv::Point3f, points>& world, const char* name) : Target(world, name) {}
	inline UniqueTarget(const std::array<cv::Point3f, points>& world, const std::string& name) : Target(world, name) {}
	inline UniqueTarget(const std::array<cv::Point3f, points>& world, std::string&& name) : Target(world, name) {}
	inline virtual ~UniqueTarget() { This_t::deleted.push(this->instance); }

private:
	inline static uint32_t assign() {
		if(This_t::deleted.empty()) {
			This_t::highest++;
			return This_t::highest;
		} else {
			uint32_t ret = This_t::deleted.front();
			This_t::deleted.pop();
			return ret;
		}
	}

	inline static std::atomic<uint32_t> highest{0};
	inline static std::queue<uint32_t> deleted;
	const uint32_t instance;


};

/** Extend this class to create a specific and reusable thresholding technique. */
class VThreshold {
public:
	VThreshold() = default;
	virtual ~VThreshold() = default;

	/** Threshold the input frame and store internally. Returns reference to internal buffer.*/
	virtual const cv::Mat& threhsold(const cv::Mat& i_frame) = 0;
	/** Sets the frame parameter equal to the last thresholded framebuffer */
	virtual void fromBinary(cv::Mat& o_frame) const = 0;
	/** Gets the downscaling factor being applied */
	virtual size_t getScale() const = 0;

	inline const cv::Mat& getBinary() const {
		return this->binary;
	}

	enum class BGR {
		BLUE = 0,
		GREEN = 1,
		RED = 2
	};

protected:
	cv::Mat binary;


};

class Contours {
public:
	Contours() = default;

	size_t findCountours(const cv::Mat& binary_frame, int mode = cv::RETR_EXTERNAL, int method = cv::CHAIN_APPROX_SIMPLE);
	size_t findLargest(const cv::Mat& binary_frame, int mode = cv::RETR_EXTERNAL, int method = cv::CHAIN_APPROX_SIMPLE);


};



}