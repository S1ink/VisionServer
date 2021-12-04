#pragma once

#include <cstdio>
#include <string>
#include <thread>
#include <vector>

#include <networktables/NetworkTableInstance.h>
#include <vision/VisionPipeline.h>
#include <vision/VisionRunner.h>
#include <wpi/StringRef.h>
#include <wpi/json.h>
#include <wpi/raw_istream.h>
#include <wpi/raw_ostream.h>

#include "cameraserver/CameraServer.h"

#include "extras/resources.h"

CE_STR default_f = "/boot/frc.json";

wpi::raw_ostream& parseError(const char* path);

struct StreamConfig;

struct CameraConfig {
	std::string name;			// the camera name
	std::string path;			// the camera path
	wpi::json config;			// the entire JSON source block for the camera
	wpi::json stream_config;	// stream config - not sure exactly what this does

	cs::UsbCamera camera;		// the camera object

	bool read(const wpi::json& source, const char* file = default_f);
	cs::MjpegServer start() const;
	bool start(const wpi::json& source, const char* file = default_f);

	CameraConfig() = default;
	CameraConfig(const wpi::json& source, const char* file = default_f);
};

struct SwitchedCameraConfig {
	std::string name;
	std::string key;

	bool read(const wpi::json& source, const char* file = default_f);
	cs::MjpegServer start(const StreamConfig& data) const;

	SwitchedCameraConfig() = default;
	SwitchedCameraConfig(const wpi::json& source, const char* file = default_f);
};

struct StreamConfig {
	uint16_t team;
	bool is_server = false;

	std::vector<CameraConfig> cameras;
	std::vector<SwitchedCameraConfig> servers;

	bool parse(const char* file = default_f);
	nt::NetworkTableInstance setup();

	StreamConfig(const char* file = default_f);
};
