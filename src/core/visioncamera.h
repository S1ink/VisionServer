#pragma once

#include <opencv2/opencv.hpp>

#include <networktables/NetworkTableInstance.h>
#include <networktables/NetworkTable.h>
#include <cameraserver/CameraServer.h>
#include <wpi/json.h>


/**
 * Adds extra functionality and ease of use on top of cs::VideoCaemra (base of cs::HTTPCamera and cs::UsbCamera)
*/
class VisionCamera : public cs::VideoCamera {
public:
	//VisionCamera() = default;
	/**
	 * Construct from a raw cscore source handle
	 * @param source_handle A cscore source handle for a camera
	*/
	VisionCamera(CS_Source source_handle);
	/**
	 * Construct from a VideoSource and optional config json
	 * @param source The source object that will be copied from
	 * @param config The config json for the camera
	*/
	VisionCamera(const cs::VideoSource& source, const wpi::json& config = wpi::json());
	/**
	 * Construct from a UsbCamera and optional config json - SEEMINGLY NOT FUNCTIONAL
	 * @param source The source object that will be copied from
	 * @param config The config json for the camera
	*/
	VisionCamera(const cs::UsbCamera& source, const wpi::json& config = wpi::json());	// these were not working
	/**
	 * Construct from a HttpCamera and optional config json - SEEMINGLY NOT FUNCTIONAL
	 * @param source The source object that will be copied from
	 * @param config The config json for the camera
	*/
	VisionCamera(const cs::HttpCamera& source, const wpi::json& config = wpi::json());	// ^
	/**
	 * Construct using json configuration and calibration blocks
	 * @param source_config The camera configuration json object
	 * @param calibration The json object containing calibration values for the camera
	*/
	VisionCamera(const wpi::json& source_config, const wpi::json& calibration);
	/**
	 * Construct using json configuration without camera calibration
	 * @param source_config The camera configuration json object
	*/
	VisionCamera(const wpi::json& source_config);
	// VisionCamera(const VisionCamera&);
	// VisionCamera(VisionCamera&&);
	/**
	 * Deletes all networktable entries for the camera's subtable
	*/
	~VisionCamera();
	//VisionCamera& operator=(const VisionCamera&);
	//VisionCamera& operator=(VisionCamera&&);

	//cs::VideoSource::Kind getType() const;
	/**
	 * Get if the cameras calibration json is valid
	 * @return Whether or not the calibration json is valid
	*/
	bool isValidJson() const;
	/**
	 * Get the calibration json
	 * @return The calibration json object
	*/
	const wpi::json& getJson() const;
	/**
	 * Get the config contains a stream configuration
	 * @return Whether or not the config json has a stream configuration block
	*/
	bool isValidStreamJson() const;
	/**
	 * Get the stream configuration json
	 * @return A stream configuration json block - returns an empty wpi::json on error (isValidStreamJson() is false)
	*/
	wpi::json getStreamJson() const;

	/**
	 * Get the camera's matrix array (doubles) - parsed from json
	 * @param array An output array that will be set to the camera's matrix values - size is 3x3 (double)
	 * @return Whether or not the array could be successfully filled
	*/
	bool getJsonCameraMatrix(cv::Mat_<double>& array) const;
	/**
	 * Get the camera's matrix array (floats) - parsed from json
	 * @param array An output array that will be set to the camera's matrix values - size is 3x3 (float)
	 * @return Whether or not the array could be successfully filled
	*/
	bool getJsonCameraMatrix(cv::Mat_<float>& array) const;
	/**
	 * Get the camera's distortion coefficients (doubles) - parsed from json
	 * @param array An output array that will be set to the camera's distortion coefficients - size is 5x1 (double)
	 * @return Whether or not the array coulb be successfully filled
	*/
	bool getJsonDistortionCoefs(cv::Mat_<double>& array) const;
	/**
	 * Get the camera's distortion coefficients (floats) - parsed from json
	 * @param array An output array that will be set to the camera's distortion coefficients - size is 5x1 (float)
	 * @return Whether or not the array could be successfully filled
	*/
	bool getJsonDistortionCoefs(cv::Mat_<float>& array) const;
	/**
	 * Get the internally stored camera matrix
	 * @return a const reference to the internal matrix
	*/
	inline const cv::Mat_<float>& getCameraMatrix() const { return this->camera_matrix; }
	/**
	 * Get the internally stored distortion coefficient matrix
	 * @return a const reference to the internal matrix
	*/
	inline const cv::Mat_<float>& getDistortionCoefs() const { return this->distortion; }

	/**
	 * Wraps cs::CvSink::GrabFrame() for the internal sink. If the camera is not physically connected, the buffer is set to a blank frame
	 * @param o_frame the output framebuffer
	 * @param timeout maximum time that the thread with block until returning with an empty frame
	 * @return the frametime, in 1 microsecond increments (see wpi::Now())
	*/
	uint64_t getFrame(cv::Mat& o_frame, double timeout = 0.225) const;
	/**
	 * Wraps cs::CvSink::GrabFrameNoTimeout() for the internal sink. If the camera is not phystically connected, the buffer is set to a blank frame
	 * @param o_frame the output framebuffer
	 * @return the frametime, in 1 microsecond increments (see wpi::Now())
	*/
	uint64_t getFrameNoTmO(cv::Mat& o_frame) const;
	/**
	 * Get the internal CvSink for aquiring frames from the camera
	 * @return a const reference to the internal sink
	*/
	inline const cs::CvSink& getSink() const { return this->source; }
	/**
	 * Creates a cs::CvSource with correct options as to be used for the camera
	 * @return A cs::CvSource object that could be used to stream frames from the camera. This object is not connected to the source feed of the camera in anyway unless passed frames are passed manually
	*/
	cs::CvSource generateServer() const;

	/**
	 * Get the width of a frame output by the camera
	 * @return The width in pixels
	*/
	int getWidth() const;
	/**
	 * Get the height of a frame output by the camera
	 * @return The height in pixels
	*/
	int getHeight() const;
	/**
	 * Get the total number of pixels in each frame (width x height)
	 * @return The total number of pixels in each frame
	*/
	int getPixels() const;
	/**
	 * Get the current fps the camera is set to capture at
	 * @return The fps
	*/
	int getConfigFPS() const;
	/**
	 * Get the width and height of a camera frame in cv::Size format
	 * @return The resolution of a camera frame in a cv::Size object
	*/
	cv::Size getResolution() const;

	/**
	 * Get the current brightness setting for the camera
	 * @return The brightness (0-100, -1 for auto)
	*/
	int8_t getBrightness() const;
	/**
	 * Get the current exposure setting for the camera
	 * @return The exposure (0-100, -1 for auto)
	*/
	int8_t getExposure() const;
	/**
	 * Get the current whitebalance setting for the camera
	 * @return The whitebalance (0-??? (>7000), -1 for auto)
	*/
	int16_t getWhiteBalance() const;

	/**
	 * Set the brightness of the camera
	 * @param b The brightness (0-100, -1 for auto)
	*/
	void setBrightness(int8_t b);		// ranges 0 through 100 (a percent)
	/**
	 * Set the whitebalance of the camera
	 * @param wb The whitebalance (0-???, -1 for auto)
	*/
	void setWhiteBalance(int16_t wb);	// ranges 0 through ~7000(+?) (look up) -> -1 sets to auto
	/**
	 * Set the exposure of the camera
	 * @param e The exposure (0-100, -1 for auto)
	*/
	void setExposure(int8_t e);		// ranges 0 through 100 -> -1 sets to auto

	/**
	 * Set the root networktable in which the camera's own networktable should reside
	 * @param table The root table (ex. "Cameras")
	*/
	void setNetworkBase(std::shared_ptr<nt::NetworkTable> table);
	/**
	 * Publish networktables-adjustable settings for exposure, brightness, and whitebalance under the camera's networktable
	*/
	void setNetworkAdjustable();
	/**
	 * Deletes all created entries in the camera's networktable
	*/
	void deleteEntries();

protected:
	/**
	 * Creates a cs::CvSink for the camera and updates stream configuration if it has any
	 * @return A cs::CvSink object connected to the current camera
	*/
	cs::CvSink getVideo() const;
	/**
	 * Publish an adjustable networktable entry for the camera's brightness
	*/
	void setBrightnessAdjustable();
	/**
	 * Publish an adjustable networktable entry for the camera's whitebalance
	*/
	void setWhiteBalanceAdjustable();
	/**
	 * Publish an adjustable networktable entry for the camera's exposure
	*/
	void setExposureAdjustable();

private:
	wpi::json config, calibration;
	cs::CvSink source;
	cv::Mat_<float> camera_matrix{cv::Mat_<float>(3, 3)}, distortion{cv::Mat_<float>(1, 5)};
	cs::VideoMode properties;

	/**The camera's networktable - default value is '(root)/Cameras/NAME/' */
	std::shared_ptr<nt::NetworkTable> camera{nt::NetworkTableInstance::GetDefault().GetTable("Cameras")->GetSubTable(this->GetName())};

	int8_t brightness{50}, exposure{-1};
	int16_t whitebalance{-1};


};