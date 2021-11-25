#pragma once

#include <networktables/NetworkTableInstance.h>
#include <networktables/NetworkTable.h>
#include <opencv2/core/types.hpp>
#include <wpi/raw_ostream.h>
#include <wpi/json.h>

#include "cameraserver/CameraServer.h"
#include "extras/resources.h"

class VisionCamera : public cs::VideoCamera {
public:
	VisionCamera() = default;
	VisionCamera(CS_Source source_handle);
	VisionCamera(const cs::VideoSource& source, const wpi::json& config = wpi::json());
	VisionCamera(const cs::UsbCamera& source, const wpi::json& config = wpi::json());	// these were not working
	VisionCamera(const cs::HttpCamera& source, const wpi::json& config = wpi::json());	// ^
	VisionCamera(const wpi::json& source_config);

	cs::VideoSource::Kind getType() const;
	bool isValidJson() const;
	wpi::json getJson() const;
	bool isValidStreamJson() const;
	wpi::json getStreamJson() const;

	cs::CvSink getVideo() const;
	cs::CvSource getSeparateServer() const;

	int getWidth() const;
	int getHeight() const;
	int getPixels() const;
	int getSetFPS() const;
	cv::Size getResolution() const;

	int8_t getBrightness() const;
	int8_t getExposure() const;
	int16_t getWhiteBalance() const;

	void setBrightness(int8_t);		// ranges 0 through 100 (a percent)
	void setWhiteBalance(int16_t);	// ranges 0 through 7000? (look up) -> -1 sets it to auto
	void setExposure(int8_t);		// ranges 0 through 100 -> -1 sets it to auto

	void setBrightnessAdjustable();
	void setBrightnessAdjustable(std::shared_ptr<nt::NetworkTable> table);
	void setWhiteBalanceAdjustable();
	void setWhiteBalanceAdjustable(std::shared_ptr<nt::NetworkTable> table);
	void setExposureAdjustable();
	void setExposureAdjustable(std::shared_ptr<nt::NetworkTable> table);

private:
	cs::VideoSource::Kind type;
	wpi::json config;
	// cs::CvSink for storing vision source?

	int8_t brightness{50}, exposure{-1};
	int16_t whitebalance{-1};
};