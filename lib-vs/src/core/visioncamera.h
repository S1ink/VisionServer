#pragma once

#include <opencv2/opencv.hpp>

#include <networktables/NetworkTableInstance.h>
#include <networktables/NetworkTable.h>
#include <networktables/IntegerTopic.h>
#include <cameraserver/CameraServer.h>
#include <wpi/json.h>


/**
 * Adds extra functionality and ease of use on top of cs::VideoCaemra (base of cs::HTTPCamera and cs::UsbCamera)
*/
class VisionCamera final: public cs::VideoCamera {
public:
	inline static const cv::Mat_<float>
		default_matrix{cv::Mat_<float>::zeros(3, 3)},
		default_distort{cv::Mat_<float>::zeros(1, 5)}
	;

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
	VisionCamera(const VisionCamera&) = delete;
	VisionCamera(VisionCamera&&);
	// inline VisionCamera(const VisionCamera&) { std::cout << "VCamera copied." << std::endl; }
	// inline VisionCamera(VisionCamera&&) { std::cout << "VCamera moved." << std::endl; }
	/**
	 * Deletes all networktable entries for the camera's subtable
	*/
	~VisionCamera();

	inline VisionCamera& operator=(const VisionCamera&) = delete;
	VisionCamera& operator=(VisionCamera&&);
	// inline VisionCamera& operator=(const VisionCamera&) { std::cout << "VCamera copied via operator." << std::endl; }
	// inline VisionCamera& operator=(VisionCamera&&) { std::cout << "VCamera moved via operator." << std::endl; }

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

	bool setCalibrationJson(const wpi::json&);
	bool setCameraMatrix(const cv::Mat_<float>&);
	bool setDistortionCoefs(const cv::Mat_<float>&);

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
	inline const cs::CvSink& getSink() const { return this->raw; }

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
	int getBrightness() const;
	/**
	 * Get the current exposure setting for the camera
	 * @return The exposure (0-100, -1 for auto)
	*/
	int getExposure() const;
	/**
	 * Get the current whitebalance setting for the camera
	 * @return The whitebalance (0-??? (>7000), -1 for auto)
	*/
	int getWhiteBalance() const;

	/**
	 * Set the brightness of the camera
	 * @param b The brightness (0-100, -1 for auto)
	*/
	void setBrightness(int b);		// ranges 0 through 100 (a percent)
	/**
	 * Set the whitebalance of the camera
	 * @param wb The whitebalance (0-???, -1 for auto)
	*/
	void setWhiteBalance(int wb);	// ranges 0 through ~7000(+?) (look up) -> -1 sets to auto
	/**
	 * Set the exposure of the camera
	 * @param e The exposure (0-100, -1 for auto)
	*/
	void setExposure(int e);		// ranges 0 through 100 -> -1 sets to auto

	/**
	 * Set the root networktable in which the camera's own networktable should reside
	 * @param table The root table (the camera resides in ~table~/Cameras)
	*/
	void setNetworkBase(const std::shared_ptr<nt::NetworkTable>& table);
	/**
	 * Publish networktables-adjustable settings for exposure, brightness, and whitebalance under the camera's networktable
	*/
	void setNetworkAdjustable();

protected:
	int _setBrightness(int b);
	int _setWhiteBalance(int wb);
	int _setExposure(int e);

private:
	wpi::json config, calibration;
	cs::CvSink raw;
	cv::Mat_<float> camera_matrix{default_matrix}, distortion{default_distort};
	cs::VideoMode properties;

	std::shared_ptr<nt::NetworkTable> ntable{
		nt::NetworkTableInstance::GetDefault().GetTable("Cameras")->GetSubTable(this->GetName())};
	nt::IntegerEntry
		nt_brightness, nt_exposure, nt_whitebalance;
	NT_Listener listener_handle{0};


};