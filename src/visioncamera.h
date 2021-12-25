#pragma once

#include <networktables/NetworkTableInstance.h>
#include <networktables/NetworkTable.h>
#include <cameraserver/CameraServer.h>
#include <wpi/json.h>

#include <opencv2/opencv.hpp>

class VisionCamera;

// VisionCamera adds extra functionality and ease of use on top of cs::VideoCaemra (base of cs::HTTPCamera and cs::UsbCamera)
class VisionCamera : public cs::VideoCamera {
public:
	//VisionCamera() = default;
	VisionCamera(CS_Source source_handle);
	VisionCamera(const cs::VideoSource& source, const wpi::json& config = wpi::json());
	VisionCamera(const cs::UsbCamera& source, const wpi::json& config = wpi::json());	// these were not working
	VisionCamera(const cs::HttpCamera& source, const wpi::json& config = wpi::json());	// ^
	VisionCamera(const wpi::json& source_config, const wpi::json& calibration);
	VisionCamera(const wpi::json& source_config);
	// VisionCamera(const VisionCamera&);
	// VisionCamera(VisionCamera&&);
	~VisionCamera();
	//VisionCamera& operator=(const VisionCamera&);
	//VisionCamera& operator=(VisionCamera&&);

	//cs::VideoSource::Kind getType() const;
	bool isValidJson() const;
	wpi::json getJson() const;
	bool isValidStreamJson() const;
	wpi::json getStreamJson() const;

	bool getCameraMatrix(cv::Mat_<double>& array) const;
	bool getCameraMatrix(cv::Mat_<float>& array) const;
	bool getDistortion(cv::Mat_<double>& array) const;
	bool getDistortion(cv::Mat_<float>& array) const;

	cs::CvSink getVideo() const;
	cs::CvSource generateServer() const;

	int getWidth() const;
	int getHeight() const;
	int getPixels() const;
	int getSetFPS() const;
	cv::Size getResolution() const;

	int8_t getBrightness() const;
	int8_t getExposure() const;
	int16_t getWhiteBalance() const;

	void setBrightness(int8_t);		// ranges 0 through 100 (a percent)
	void setWhiteBalance(int16_t);	// ranges 0 through ~7000(+?) (look up) -> -1 sets to auto
	void setExposure(int8_t);		// ranges 0 through 100 -> -1 sets to auto

	void setNetworkBase(std::shared_ptr<nt::NetworkTable> table);
	void setNetworkAdjustable();
	void deleteEntries();

protected:
	void setBrightnessAdjustable();
	void setWhiteBalanceAdjustable();
	void setExposureAdjustable();

private:
	//cs::VideoSource::Kind type;	// does this even do anything?
	wpi::json config, calibration;

	std::shared_ptr<nt::NetworkTable> camera{nt::NetworkTableInstance::GetDefault().GetTable("Cameras")->GetSubTable(this->GetName())};

	int8_t brightness{50}, exposure{-1};
	int16_t whitebalance{-1};
};