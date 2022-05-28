#pragma once

#include <opencv2/opencv.hpp>

#include <vector>
#include <array>
#include <thread>
#include <string>

#include "../core/visionserver2.h"
#include "../core/target.h"


class UpperHub : public vs2::Target {
	friend class UHPipeline;
public:
	inline static const std::array<cv::Point3f, 6> world_coords{	// left to right
		cv::Point3f(0.f, 103.f, 26.73803044f),				// @c=0
		cv::Point3f(10.23220126f, 103.f, 24.70271906f),		// @c=10.5
		cv::Point3f(18.90664264f, 103.f, 18.90664264f),		// @c=21
		cv::Point3f(24.70271906f, 103.f, 10.23220126f),		// @c=31.5
		cv::Point3f(26.73803044f, 103.f, 0.f),				// @c=42
		cv::Point3f(24.70271906f, 103.f, -10.23220126f),	// @c=52.5
	};
	inline static const size_t min = 3, max = 6;
	
	inline UpperHub() : Target("Upper Hub") {}
	UpperHub(const UpperHub&) = delete;


};
class UHPipeline : public vs2::VPipeline<UHPipeline> {
public:
	UHPipeline() = default;
	UHPipeline(const UHPipeline&) = delete;

	void process(cv::Mat& io_frame) override;

private:	// currently the starting size is 840 bytes
// thresholding
	std::array<cv::Mat, 3> channels;
	cv::Mat binary, buffer;
	double alpha{0.5}, beta{0.5}, gamma{0.0};
	uint8_t thresh{50};
	size_t scale{1};
// contours
	std::vector<std::vector<cv::Point2i> > contours;
	double area_largest{0.f}, area_buff{0.f};
	int16_t target_idx{-1};
// algorithm
	std::vector<cv::Rect> in_range;
	cv::Rect rect_buff;
	cv::Size range_buff;
	UpperHub target;
// pose solving
	std::vector<cv::Point2f> points_buff;
	std::vector<cv::Point3f> world_buff;
	std::array<cv::Mat_<float>, 2> tvecs, rvecs;
	cv::Mat_<float>
		rvec = cv::Mat_<float>(1, 3),
		tvec = rvec
	;


};


enum class CargoColor {
	NONE = 0b00,
	RED = 0b01,
	BLUE = 0b10,
	BOTH = 0b11
};
template<typename t = uint8_t>
inline t operator~(CargoColor c) { return static_cast<t>(c); }

template<typename t = float>
struct CvCargo {
	CvCargo() = default;
	CvCargo(cv::Point2_<t> c, t r) : center(c), radius(t) {}
	CvCargo(cv::Point2_<t> c, t r, CargoColor color) : center(c), radius(t), color(color) {}

	cv::Point2_<t> center;
	t radius{(t)0.0};
	CargoColor color{CargoColor::NONE};

};

template<CargoColor color = CargoColor::NONE>
class Cargo : public vs2::UniqueTarget<Cargo<color>> {
	friend class CargoPipeline;
	typedef struct Cargo<color>		This_t;
public:
	inline static const std::array<cv::Point3f, 4> world_coords{
		cv::Point3f(-4.75f, 0.f, 0.f),
		cv::Point3f(0.f, 4.75f, 0.f),
		cv::Point3f(4.75f, 0.f, 0.f),
		cv::Point3f(0.f, -4.75f, 0.f)
	};
	inline static const std::array<const char*, 4> name_map{
		"Extra Cargo #",
		"Red Cargo #",
		"Blue Cargo #",
		"Extra Cargo #"
	};

	Cargo() : vs2::UniqueTarget<This_t>(name_map[~color]) {}
	Cargo(const Cargo&) = delete;

	void update(const CvCargo& v);


};
class CargoPipeline : public vs2::VPipeline<CargoPipeline> {
public:
	CargoPipeline() = default;
	CargoPipeline(const CargoPipeline&) = delete;

	void process(cv::Mat& io_frame) override;

protected:



};