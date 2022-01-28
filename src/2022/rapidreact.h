#pragma once

#include <opencv2/opencv.hpp>

#include <vector>
#include <array>

#include "../api/weightedsubtraction.h"
#include "../api/visionserver.h"
#include "../api/processing.h"
#include "../api/target.h"

// template<typename _Tp>
// class Tile_ : public cv::Rect_<_Tp> {
// public:
// 	using Rect_::Rect_;

// 	inline cv::Point2f getCenter() const { return ::findCenter<float, _Tp>({this->tl(), this->br()}); }
// 	inline float getAspectRatio() const { return this->width/(float)this->height; }

// };
// typedef Tile_<int> Tile2i;
// typedef Tile_<float> Tile2f;
// typedef Tile_<double> Tile2d;
// typedef Tile2i Tile;

/**
 * Models 3-6 of the closest 5(or 4) vision strips on the Upper Hub
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
class UpperHub : public Target<0> {
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
		cv::Point3f(24.70271906f, 103.f, -10.23220126f),		// @c=52.5
	};

};
template<VThreshold::LED color>
class StripFinder : public VPipeline, public WeightedSubtraction<color>, public Contours {
public:
	StripFinder(VisionServer& server);
	StripFinder(const StripFinder& other) = delete;

	void process(cv::Mat& io_frame, int8_t mode = 0) override;

private:
	std::vector<cv::Rect> in_range;
	cv::Rect rect_buffer;
	cv::Size range;

	UpperHub target;

	std::array<cv::Point3f, 2> pose3D{cv::Point3f(0.f, 103.f, 0.f), cv::Point3f(0.f, 0.f, 0.f)};
	std::array<cv::Point2f, 2> pose2D;

	cv::Mat_<float> rvec = cv::Mat_<float>(1, 3), tvec = rvec/*, rmat = cv::Mat_<float>(3, 3)*/;

};

enum class CargoColor {
	NONE = 0b00,
	RED = 0b01,
	BLUE = 0b10
};
struct CargoOutline {
	CargoOutline() {}
	CargoOutline(cv::Point c, double r) : center(c), radius(r) {}
	CargoOutline(cv::Point c, double r, CargoColor color) : center(c), radius(r), color(color) {}

	cv::Point2f center;
	float radius{0.0};
	CargoColor color{CargoColor::NONE};
};

class Cargo : public Target<3> {
public:
	Cargo(size_t ball_num, CargoColor color) : Target<3>({	// in inches
		cv::Point3f(-4.75f, 0.f, 0.f),
		cv::Point3f(0.f, 4.75f, 0.f),
		cv::Point3f(4.75f, 0.f, 0.f)
	}, "Cargo-" + std::to_string(ball_num) + ((uint)color == 1 ? 'r' : 'b')) {}
	Cargo(const Cargo& other) = delete;

	template<typename num_t>
	inline bool compatible(const std::vector<cv::Point_<num_t> >& contour) const { return this->size() == contour.size(); }
	// template<typename num_t>
	// void sort(std::vector<cv::Point_<num_t> >& points);
	//template<typename num_t>
	void sort(CargoOutline outline);

	// void solvePerspective(
	// 	cv::Mat_<float>& tvec, cv::Mat_<float>& rvec,
	// 	cv::InputArray camera_matrix, cv::InputArray camera_coeffs,
	// 	int flags = cv::SOLVEPNP_ITERATIVE, bool ext_guess = false
	// );

private:

};
class CargoFinder : public VPipeline, public WeightedSubtraction<VThreshold::LED::RED> {
public:
	CargoFinder(VisionServer& server);
	CargoFinder(const CargoFinder& other) = delete;

	void process(cv::Mat& io_frame, int8_t mode = 0) override;

protected:
	class RedFinder : public Contours {
	public:
		RedFinder(CargoFinder& env) : env(&env) {}

		void threshold();

		std::vector<std::vector<cv::Point> > filtered;
		CargoFinder* env;

	} red;
	class BlueFinder : public Contours {
	public:
		BlueFinder(CargoFinder& env) : env(&env) {}

		void threshold();

		std::vector<std::vector<cv::Point> > filtered;
		CargoFinder* env;

	} blue;

	//std::vector<Cargo> balls;
	std::vector<CargoOutline> filtered;
	std::vector<cv::Point> point_buffer;
	CargoOutline outline_buffer;
	double max_val = 0.0;
	
};

#include "rapidreact.inc"