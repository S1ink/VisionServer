#pragma once

#include <vector>
#include <array>
#include <queue>
#include <string>
#include <type_traits>

#include <opencv2/opencv.hpp>

#include <networktables/NetworkTable.h>

#include "tools/src/types.h"
#include "visionserver2.h"


namespace vs2 {

template<size_t points>
class Target {
public:
	inline static const std::shared_ptr<nt::NetworkTable>
		target_table{VisionServer::base_table->GetSubTable("Targets")};

	Target() = delete;
	inline Target(std::array<cv::Point3f, points>&& world, const char* name) :
		world(std::move(world)), name(name), table(target_table->GetSubTable(this->name)) {}
	inline Target(std::array<cv::Point3f, points>&& world, const std::string& name) :
		world(std::move(world)), name(name), table(target_table->GetSubTable(this->name)) {}
	inline Target(std::array<cv::Point3f, points>&& world, std::string&& name) :
		world(std::move(world)), name(std::move(name)), table(target_table->GetSubTable(this->name)) {}
	virtual ~Target();

	const std::array<cv::Point3f, points> world;
	static inline const size_t size{points};

	inline const std::string& getName() const { return this->name; }
	inline const std::array<cv::Point2f, points>& getBuffer() const { return this->point_buff; }

	virtual void solvePerspective (
		cv::Mat_<float>& tvec, cv::Mat_<float>& rvec,
		cv::InputArray camera_matrix, cv::InputArray camera_coeffs,
		int flags = 0, bool ext_guess = false
	) const;

protected:
	void rescaleBuffer(double scale);	// all points are multiplied by scale, so to downscale by (ex. 4), pass in (ex. 0.25)
	virtual void update2D(const std::vector<cv::Point>& pts) = 0;	// a method for updating the buffer from a contour, implementations should also sort the points.

	const std::string name;
	const std::shared_ptr<nt::NetworkTable> table;

	std::array<cv::Point2f, points> point_buff;


};
/** The same target base class but with instance counting (appended to name on creation) */
template<size_t points, class derived = void>
class UniqueTarget : public Instanced<UniqueTarget<points, derived> >, public Target<points> {
	typedef struct UniqueTarget<points, derived>	This_t;
public:
	UniqueTarget() = delete;
	inline UniqueTarget(std::array<cv::Point3f, points>&& world, const char* name) :
		Instanced<This_t>(), Target<points>(world, name + std::to_string(this->getInst())) {}
	inline UniqueTarget(std::array<cv::Point3f, points>&& world, const std::string& name) :
		Instanced<This_t>(), Target<points>(world, name + std::to_string(this->getInst())) {}
	inline UniqueTarget(std::array<cv::Point3f, points>&& world, std::string&& name) :
		Instanced<This_t>(), Target<points>(world, name + std::to_string(this->getInst())) {}


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



} // namespace vs2



// extensions.inc	>>>

namespace vs2 {

template<size_t points>
void Target<points>::solvePerspective(
	cv::Mat_<float>& tvec, cv::Mat_<float>& rvec, 
	cv::InputArray camera_matrix, cv::InputArray camera_coeffs, 
	int flags, bool ext_guess
) const {
	cv::solvePnP(this->world, this->points, camera_matrix, camera_coeffs, rvec, tvec, ext_guess, flags);

	this->table->PutNumber("x", tvec[0][0]);
	this->table->PutNumber("y", tvec[1][0]);
	this->table->PutNumber("z", tvec[2][0]);
	this->table->PutNumber("distance", sqrt(pow(tvec[0][0], 2) + pow(tvec[1][0], 2) + pow(tvec[2][0], 2)));
	this->table->PutNumber("up-down", atan2(tvec[1][0], tvec[2][0])*-180/M_PI);
	this->table->PutNumber("left-right", atan2(tvec[0][0], tvec[2][0])*180/M_PI);
}
template<size_t points>
void Target<points>::rescaleBuffer(double scale) {
	for(size_t i = 0; i < points; i++) {
		this->point_buff.at(i) *= scale;
	}
}

}