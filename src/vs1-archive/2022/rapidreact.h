#pragma once

#include <opencv2/opencv.hpp>

#include <vector>
#include <array>
#include <thread>

#include "../weightedsubtraction.h"
#include "../visionserver.h"
#include "../processing.h"
#include "../target.h"
#include "../../core/vision.h"


/**
 * Models the closest 4-6 vision strips on the Upper Hub
 * 
 * - The diameter is 4ft,5.375in (427/8 or 53.375 inches) -> radius is 427/16 or 26.6875 inches
 * - Each strip is 5in, with 5.5in separation (along the circumfrance) -> also 2in high
 * - The total lenght of all strips and separation does not (exactly) line up with the cirumfrance if the diameter is 26.6875 inches (~167.6825 compared with 168)
 * - With account for the thickness of the tape itself(~1/50in), we can make the measurements align better and just call the radius 168/2/pi (~26.73803044 inches)
 * - The x and z points are found by:
 * 		x: r*sin(c/r), where r is the radius(26.6875 ~26.73803044) and c is the midpoint of a strip along the circumfrance (see "@c=" below)
 * 		z: r*cos(c/r), where r is the radius(26.6875 ~26.73803044) and c is the midpoint of a strip along the circumfrance (see "@c=" below)
 * - The height from the floor to the top of the targets is 8ft,8in, but since the target strips are 2inches, the vision midpoints would be at ~8ft,7in -> 103 inches (y-coord)
 * 
 * See here: https://www.desmos.com/calculator/4onhrkdwhg
*/
class UpperHub : public vs1::Target<0> {
	template<vs1::VThreshold::LED color> friend class StripFinder;
public:
	UpperHub() : Target<0>({}, "Upper-Hub"), points(6), world_match(6) {}
	UpperHub(const UpperHub&) = delete;
	
	inline size_t size() { return this->points.size(); }
	inline void add(const cv::Point2f& point) { this->points.push_back(point); }
	inline void add(cv::Point2f&& point) { this->points.emplace_back(point); }
	inline const std::vector<cv::Point2f>& getPoints() const { return this->points; }

	inline bool compatible() const { return ::inRange(this->points.size(), 3U, 6U); }
	inline bool compatible(const std::vector<cv::Point>& contour) const { return ::inRange(contour.size(), 3U, 6U); }

	inline void resetPoints() { this->points.clear(); }

	void sort();
	template<typename num_t>
	void sort(std::vector<cv::Rect_<num_t> >& contour);
	template<typename num_t> 
	void sort(const std::vector<cv::Point_<num_t> >& contour);

	//void rescale(double scale);
	//std::vector<cv::Point2f> getRescaled(double scale) const;

	void solvePerspective(
		cv::Mat_<float>& tvec, cv::Mat_<float>& rvec,
		cv::InputArray camera_matrix, cv::InputArray camera_coeffs,
		int flags = cv::SOLVEPNP_ITERATIVE, bool ext_guess = false
	);

protected:
	void matchWorld();

private:
	std::array<cv::Mat, 2> tvecs, rvecs;
	//cv::Mat_<float> rmat, pzero_world;
	// cv::Mat_<float>
	// 	tstart = (cv::Mat_<float>(1, 3) << -5.377486299340092, 87.15036371545433, 124.0995064393181),
	// 	rstart = (cv::Mat_<float>(1, 3) << 2.928241467489054, -0.2228350190508827, -0.05335716651782262);

	std::vector<cv::Point2f> points;
	std::vector<cv::Point3f> world_match;
	const std::array<cv::Point3f, 6> world{		// left to right
	// according to game manual
		// cv::Point3f(0.f, 103.f, 26.6875f),				// @c=0
		// cv::Point3f(10.23119406f, 103.f, 24.64843452f),	// @c=10.5
		// cv::Point3f(18.89895395f, 103.f, 18.84282879f),	// @c=21
		// cv::Point3f(24.67875472f, 103.f, 10.1578404f),	// @c=31.5
		// cv::Point3f(26.68738197f, 103.f, -0.0793729116f),	// @c=42
		// cv::Point3f(24.61789629f, 103.f, -10.30445721f),	// @c=52.5
	// exact, 168in circumfrance
		cv::Point3f(0.f, 103.f, 26.73803044f),				// @c=0
		cv::Point3f(10.23220126f, 103.f, 24.70271906f),		// @c=10.5
		cv::Point3f(18.90664264f, 103.f, 18.90664264f),		// @c=21	// perfectly 45*, unlike above
		cv::Point3f(24.70271906f, 103.f, 10.23220126f),		// @c=31.5
		cv::Point3f(26.73803044f, 103.f, 0.f),				// @c=42	// perfectly 90*, unlike above
		cv::Point3f(24.70271906f, 103.f, -10.23220126f),	// @c=52.5
	};
	// const std::array<float, 6> angles {	// in radians -> half of the angle between 0 and the point at the corresponding index ^
	// 	0.f,
	// 	0.196349540849f,
	// 	0.392699081699f,
	// 	0.589048622548f,
	// 	0.785398163397f,
	// 	0.981747704247f
	// };

};

template<vs1::VThreshold::LED color>
class StripFinder : public vs1::VPipeline, public vs1::WeightedSubtraction<color>, public vs1::Contours {
public:
	StripFinder(vs1::VisionServer& server);
	StripFinder(const StripFinder& other) = delete;

	void process(cv::Mat& io_frame) override;

private:
	std::vector<cv::Rect> in_range;
	cv::Rect rect_buffer;
	cv::Size range;

	UpperHub target;

	std::array<cv::Point3f, 4> pose3D{cv::Point3f(0.f, 0.f, 0.f), cv::Point3f(0.f, 103.f, 0.f), cv::Point3f(50.f, 103.f, 0.f), cv::Point3f(0.f, 103.f, 50.f)};
	std::array<cv::Point2f, 4> pose2D;

	cv::Mat_<float> rvec = cv::Mat_<float>(1, 3), tvec = rvec/*, rmat = cv::Mat_<float>(3, 3)*/;

};

enum class CargoColor {
	NONE = 0b00,
	RED = 0b01,
	BLUE = 0b10,
	BOTH = 0b11
};
template<typename num_t>
inline num_t operator~(CargoColor color) { return static_cast<num_t>(color); }
inline uint8_t operator~(CargoColor color) { return static_cast<uint8_t>(color); }

struct CargoOutline {
	CargoOutline() {}
	CargoOutline(cv::Point c, double r) : center(c), radius(r) {}
	CargoOutline(cv::Point c, double r, CargoColor color) : center(c), radius(r), color(color) {}

	cv::Point2f center;
	float radius{0.0};
	CargoColor color{CargoColor::NONE};
};

class Cargo : public vs1::Target<4> {
public:
	Cargo(size_t ball_idx, CargoColor color) : Target<4>({	// in inches
		cv::Point3f(-4.75f, 0.f, 0.f),
		cv::Point3f(0.f, 4.75f, 0.f),
		cv::Point3f(4.75f, 0.f, 0.f),
		cv::Point3f(0.f, -4.75f, 0.f)
	}, "Cargo-" + std::to_string(ball_idx) + (~color == 1 ? 'r' : 'b')) {}
	Cargo(const Cargo& other) = delete;

	void sort(CargoOutline outline);

};
#ifdef TENNIS_DEMO
class Tennis : public Target<4> {
public:
	Tennis(size_t ball_idx) : Target<4>({
		cv::Point3f(-1.29f, 0.f, 0.f),
		cv::Point3f(0.f, 1.29f, 0.f),
		cv::Point3f(1.29f, 0.f, 0.f),
		cv::Point3f(0.f, -1.29f, 0.f)
	}, "Normal-Sized-" + std::to_string(ball_idx)) {}
	Tennis(const Tennis& other) = delete;

	void sort(CargoOutline outline);
};
#endif

class CargoFinder : public vs1::VPipeline {
public:
	CargoFinder(vs1::VisionServer& server);
	CargoFinder(const CargoFinder& other) = delete;

	void process(cv::Mat& io_frame) override;

protected:
	template<vs1::VThreshold::LED primary, uint8_t alpha, uint8_t beta, uint8_t thresh_percent>
	class BallFilter : public vs1::WeightedSubtraction<primary>, public vs1::Contours {
		friend class CargoFinder;
	public:
		BallFilter(vs1::VisionServer& server, CargoFinder& env);
		BallFilter(const BallFilter& other) = delete;

		void threshold(cv::Mat& o_frame, const std::array<cv::Mat, 3>& channels);
		bool launchThresholding(cv::Mat& o_frame, const std::array<cv::Mat, 3>& channels);
		bool join();

	protected:
		static inline void thresholdWorker(BallFilter* that, cv::Mat& o_frame, const std::array<cv::Mat, 3>& channels) { that->threshold(o_frame, channels); }
		inline void resetTargetIdx() { this->target_idx = -1; }

		std::vector<CargoOutline> filtered;
		std::thread worker;

	private:
		std::vector<cv::Point> point_buffer;
		CargoOutline outline_buffer;
		double max_val{0.0};
		CargoFinder* env;

	};
	BallFilter<vs1::VThreshold::LED::RED, 100, 100, 30> red;
	BallFilter<vs1::VThreshold::LED::BLUE, 20, 100, 15> blue;

	cv::Mat_<float> rvec = cv::Mat_<float>(1, 3), tvec = rvec/*, rmat = cv::Mat_<float>(3, 3)*/;
	Cargo red_c{1, CargoColor::RED}, blue_c{1, CargoColor::BLUE};

#ifdef TENNIS_DEMO
	BallFilter<VThreshold::LED::GREEN, 100, 15, 15> normal;
	Tennis tennis{1};
#endif
	
};

#include "rapidreact.inc"