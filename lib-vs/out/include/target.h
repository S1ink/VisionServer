#pragma once

#include <vector>
#include <array>
#include <string>

#include <opencv2/opencv.hpp>

#include <networktables/NetworkTable.h>

#include "cpp-tools/src/types.h"
#include "visionserver2.h"


namespace vs2 {

/**
 * Basic ntables target representation
*/
class Target {
public:
	// inline static const std::shared_ptr<nt::NetworkTable>
	// 	target_table{VisionServer::getTable()->GetSubTable("Targets")};

	Target() = delete;
	inline Target(const std::string& n) : name(n), table(VisionServer::targetsTable()->GetSubTable(this->name)) {}
	inline Target(std::string&& n) : name(std::move(n)), table(VisionServer::targetsTable()->GetSubTable(this->name)) {}
	Target(const Target&) = default;
	Target(Target&&) = default;

	inline const std::string& getName() const { return this->name; }

protected:
	inline void setPos(double x, double y, double z) {
		this->table->PutNumber("x", x);
		this->table->PutNumber("y", y);
		this->table->PutNumber("z", z);
	}
	inline void setAngle(double ud, double lr) {
		this->table->PutNumber("up-down", ud);
		this->table->PutNumber("left-right", lr);
	}
	inline void setDist(double d) {
		this->table->PutNumber("distance", d);
	}

	const std::string name;
	const std::shared_ptr<nt::NetworkTable> table;


};
/**
 * UniqueTarget<> must be extended by another class so that names match instancing types.
*/
template<class derived = void>
class UniqueTarget : public Instanced<UniqueTarget<derived> >, public Target {
	typedef struct UniqueTarget<derived>	This_t;
public:
	UniqueTarget() = delete;
	inline UniqueTarget(const std::string& n) : Instanced<This_t>(), Target(n + std::to_string(this->getInst())) {}
	inline UniqueTarget(std::string&& n) : Instanced<This_t>(), Target(n + std::to_string(this->getInst())) {}
	UniqueTarget(const UniqueTarget&) = default;
	UniqueTarget(UniqueTarget&&) = default;


};

// /**
//  * A target that is generated from OpenCV pipelines (and solved using solvePnP)
// */
// template<size_t points>
// class CvTarget : public Target {
// public:
// 	const std::array<cv::Point3f, points> world;
// 	static inline const size_t size{points};

// 	CvTarget() = delete;
// 	inline CvTarget(std::array<cv::Point3f, points>&& world, const std::string& name) :
// 		Target(name), world(world) {}
// 	inline CvTarget(std::array<cv::Point3f, points>&& world, std::string&& name) :
// 		Target(std::move(name)), world(world) {}
// 	CvTarget(const CvTarget&) = delete;
// 	CvTarget(CvTarget&&) = delete;

// protected:
// 	std::array<cv::Point2f, points> point_buff;


// };
// /**
//  * A target that is generated from TensorFlow pipelines
// */
// class TfTarget : public Target {

// };

}	// namespace vs2