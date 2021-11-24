#pragma once

#include <networktables/NetworkTable.h>
#include <opencv2/core/types.hpp>

#include "cameraserver/CameraServer.h"
#include "extras/resources.h"

class VisionCamera : public cs::VideoCamera {
public:
	VisionCamera() = default;
	VisionCamera(const cs::VideoSource& source);

	int getWidth();
	int getHeight();
	int getPixels();
	int getSetFPS();
	cv::Size getResolution();

	int8_t getBrightness();
	int8_t getExposure();
	int16_t getWhiteBalance();

	void setBrightness(int8_t);
	void setWhiteBalance(int16_t);
	void setExposure(int8_t);

	void setBrightnessAdjustable(std::shared_ptr<nt::NetworkTable> table);
	void setWhiteBalanceAdjustable(std::shared_ptr<nt::NetworkTable> table);
	void setExposureAdjustable(std::shared_ptr<nt::NetworkTable> table);

private:
	int8_t brightness{0}, exposure{0};
	int16_t whitebalance{0};
};