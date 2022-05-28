#pragma once

#include <vector>
#include <array>
#include <string>
#include <type_traits>

#include <opencv2/opencv.hpp>

#include <networktables/NetworkTable.h>

#include "tools/src/types.h"
#include "visionserver.h"
#include "processing.h"


namespace vs1 {

template<size_t corners>
class Target {
public:
	Target(const std::array<cv::Point3f, corners>& world_pts);
	Target(const std::array<cv::Point3f, corners>& world_pts, const char* name);
	Target(const std::array<cv::Point3f, corners>& world_pts, const std::string& name);
	//virtual ~Target() {}	// delete nt entries

	std::array<cv::Point2f, corners> points;		// actual points 
	const std::array<cv::Point3f, corners> world;	// world points that relate to the above 

	static inline size_t size() { return corners; }
	inline const cv::Point2f& getCenter() { return this->center; }
	inline const std::string& getName() const { return this->name; }
	template<typename num_t>
	inline bool compatible(const std::vector<cv::Point_<num_t> >& contour) const { return contour.size() == corners; }

	template<typename num_t> 
	void sort(const std::vector<cv::Point_<num_t> >& contour);

	void rescale(double scale);
	std::array<cv::Point2f, corners> getRescaled(double scale) const;

	void solvePerspective(
		cv::Mat_<float>& tvec, cv::Mat_<float>& rvec, 
		cv::InputArray camera_matrix, cv::InputArray camera_coeffs, 
		int flags = 0, bool ext_guess = false
	);

protected:
	inline const std::shared_ptr<nt::NetworkTable>& getTable() const { return this->table; }

	cv::Point2f center, a, b;

private:
	const std::string name;
	const std::shared_ptr<nt::NetworkTable> table;

};

// template<size_t pts_odd, size_t pts_even>
// class MultiTarget : public Target<pts_odd>, public Target<pts_even> {
// public:
// 	Target2(const std::array<cv::Point3f, pts_odd>& world_1, const std::array<cv::Point3f, pts_even>& world_2);
// 	Target2(const std::array<cv::Point3f, pts_odd>& world_1, const std::array<cv::Point3f, pts_even>& world_2, const char* name);
// 	Target2(const std::array<cv::Point3f, pts_odd>& world_1, const std::array<cv::Point3f, pts_even>& world_2, const std::string& name);

// };



template<class target_t, class threshold_t>
class TargetSolver : public VPipeline, public threshold_t, public Contours {
	static_assert(is_base_of_num_template<target_t, Target>::value, "Target type (target_t) must inherit from Target<size_t>");
	static_assert(std::is_base_of<VThreshold, threshold_t>::value, "Threshold type (threhsold_t) must inherit from VThreshold");
public:
	TargetSolver(VisionServer& server);
	TargetSolver(const TargetSolver&) = delete;
	//TargetSolver(TargetSolver&&);
	~TargetSolver();
	TargetSolver& operator=(const TargetSolver&) = delete;
	//TargetSolver& operator=(TargetSolver&&);

	void process(cv::Mat& io_frame) override;

private:
	std::vector<cv::Point> target_points;
	target_t reference_points;

	std::array<cv::Point3f, 1> pose3D{cv::Point3f(0, 0, 0.25)};
	std::array<cv::Point2f, 1> pose2D;

	cv::Mat_<float> rvec = cv::Mat_<float>(1, 3), tvec = rvec/*, rmat = cv::Mat_<float>(3, 3)*/;

};

}	// namespace vs1;

#include "target.inc"