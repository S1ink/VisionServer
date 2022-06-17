#pragma once

#include <vector>
#include <array>
#include <thread>
#include <string>
#include <type_traits>

#include <opencv2/opencv.hpp>

#include "../core/visionserver2.h"
#include "../core/extensions.h"
#include "../core/target.h"


class UHPipeline : public vs2::VPipeline<UHPipeline> {
public:
	UHPipeline(vs2::BGR c = vs2::BGR::GREEN);
	UHPipeline(const UHPipeline&) = delete;

	virtual void process(cv::Mat& io_frame) override;

private:	// currently the starting size is 848 bytes
// thresholding
	std::array<cv::Mat, 3> channels;
	cv::Mat binary, buffer;
	double alpha{0.5}, beta{0.5}, gamma{0.0};	// add ntable options
	uint8_t thresh{50};
	size_t scale{1};
	vs2::BGR color;
// contours
	std::vector<std::vector<cv::Point2i> > contours;
	double area_largest{0.f}, area_buff{0.f};
	int16_t target_idx{-1};
// algorithm
	std::vector<cv::Rect> in_range;
	cv::Rect rect_buff;
	cv::Size range_buff;
	struct UpperHub : public vs2::Target {
	friend class UHPipeline;
		inline static const size_t min = 4, max = 6;
		inline static const std::array<cv::Point3f, 6> world_coords{	// left to right
			cv::Point3f(0.f, 103.f, 26.73803044f),				// @c=0
			cv::Point3f(10.23220126f, 103.f, 24.70271906f),		// @c=10.5
			cv::Point3f(18.90664264f, 103.f, 18.90664264f),		// @c=21
			cv::Point3f(24.70271906f, 103.f, 10.23220126f),		// @c=31.5
			cv::Point3f(26.73803044f, 103.f, 0.f),				// @c=42
			cv::Point3f(24.70271906f, 103.f, -10.23220126f),	// @c=52.5
		};
		
		inline UpperHub() : Target("Upper Hub") {}
		UpperHub(const UpperHub&) = delete;
	} target;
// pose solving
	std::vector<cv::Point2f> points_buff;
	std::vector<cv::Point3f> world_buff;
	std::array<cv::Mat, 2> tvecs, rvecs;
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
struct CvCargo_ {
	inline CvCargo_() {
		static_assert(std::is_arithmetic<t>::value, "Template paramter t must be arithmetic type.");
	}
	inline CvCargo_(cv::Point_<t> c, t r) : center(c), radius(r) {}
	inline CvCargo_(cv::Point_<t> c, t r, CargoColor color) : center(c), radius(r), color(color) {}

	cv::Point_<t> center;
	t radius{(t)0.0};
	CargoColor color{CargoColor::NONE};

};
typedef CvCargo_<>	CvCargo;

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
		"Random Cargo #"
	};

	Cargo() : vs2::UniqueTarget<This_t>(name_map[~color]) {}
	Cargo(const Cargo&) = delete;

	template<typename t = float>
	void update(const CvCargo_<t>& v, cv::InputArray matx, cv::InputArray dist);


};
typedef Cargo<CargoColor::RED>	RedCargo;
typedef Cargo<CargoColor::BLUE>	BlueCargo;

class CargoPipeline : public vs2::VPipeline<CargoPipeline> {
public:
	CargoPipeline() = default;
	CargoPipeline(const CargoPipeline&) = delete;

	void process(cv::Mat& io_frame) override;

protected:
	template<vs2::BGR base, uint8_t a, uint8_t b, uint8_t thresh>
	class CargoFilter {
		friend class CargoPipeline;
	public:
		void threshold(const std::array<cv::Mat, 3>& channels, cv::Mat& o_frame);

	protected:
		cv::Mat binary;
		std::vector<std::vector<cv::Point2i> > contours;
		std::vector<cv::Point2i> point_buff;
		double area_largest, area_buff, max_val;
		int16_t target_idx;

		std::vector<CvCargo> objects;

	};

	std::array<cv::Mat, 3> channels;
	cv::Mat buffer;

template<CargoColor key, vs2::BGR base, uint8_t a, uint8_t b, uint8_t thresh>
using CargoPair = std::pair<std::vector<Cargo<key> >, CargoFilter<base, a, b, thresh> >;

	CargoPair<CargoColor::RED, vs2::BGR::RED, 100, 100, 30> red;
	CargoPair<CargoColor::BLUE, vs2::BGR::BLUE, 20, 100, 15> blue;


};