#include "visioncamera.h"

#include <wpi/raw_ostream.h>

#include "cpp-tools/src/resources.h"
#include "vision.h"


VisionCamera::VisionCamera(CS_Source handle) : 
	VideoCamera(handle), raw(this->GetName() + "_cvraw"), properties(this->GetVideoMode())
{
    this->raw.SetSource(*this);
}
VisionCamera::VisionCamera(const cs::VideoSource& source, const wpi::json& config) : 
	VideoCamera(source.GetHandle()), config(config), raw(this->GetName() + "_cvraw"), properties(this->GetVideoMode())
{
    if(this->properties) { this->properties = ::getJsonVideoMode(config); }
    this->raw.SetSource(*this);
}
VisionCamera::VisionCamera(const cs::UsbCamera& source, const wpi::json& config) : 
	VideoCamera(source.GetHandle()), config(config), raw(this->GetName() + "_cvraw"), properties(this->GetVideoMode())
{
    if(this->properties) { this->properties = ::getJsonVideoMode(config); }
    this->raw.SetSource(*this);
}
VisionCamera::VisionCamera(const cs::HttpCamera& source, const wpi::json& config) :
	VideoCamera(source.GetHandle()), config(config), raw(this->GetName() + "_cvraw"), properties(this->GetVideoMode())
{
    if(this->properties) { this->properties = ::getJsonVideoMode(config); }
    this->raw.SetSource(*this);
}
VisionCamera::VisionCamera(const wpi::json& source_config, const wpi::json& calibration) :
    config(source_config), calibration(calibration)
{
	cs::UsbCamera cam;
	try {cam = cs::UsbCamera(source_config.at("name").get<std::string>(), source_config.at("path").get<std::string>());}
	catch (const wpi::json::exception& e) {
		wpi::errs() << "Config error in source JSON -> could not read camera name and/or path: " << e.what() << newline;
	}
	cam.SetConfigJson(source_config);
	cam.SetConnectionStrategy(cs::VideoSource::kConnectionKeepOpen);
	// print confirmation
	swap(*this, cam);	// this should work
    this->getJsonCameraMatrix(this->camera_matrix);
    this->getJsonDistortionCoefs(this->distortion);
    this->raw = cs::CvSink(this->GetName() + "_cvraw");
    this->raw.SetSource(*this);
    this->properties = this->GetVideoMode();
    if(this->properties) { this->properties = ::getJsonVideoMode(config); }
}
VisionCamera::VisionCamera(const wpi::json& source_config) :
    config(source_config)
{
	cs::UsbCamera cam;
	try {cam = cs::UsbCamera(source_config.at("name").get<std::string>(), source_config.at("path").get<std::string>());}
	catch (const wpi::json::exception& e) {
		wpi::errs() << "Config error in source JSON -> could not read camera name and/or path: " << e.what() << newline;
	}
	cam.SetConfigJson(source_config);
	cam.SetConnectionStrategy(cs::VideoSource::kConnectionKeepOpen);
	// print confirmation
	swap(*this, cam);	// this should work
    this->raw = cs::CvSink(this->GetName() + "_cvraw");
    this->raw.SetSource(*this);
    this->properties = this->GetVideoMode();
    if(!this->properties) { this->properties = ::getJsonVideoMode(config); }
}
// VisionCamera::VisionCamera(const VisionCamera& other) : 
//     VideoCamera(other.GetHandle())/*, type(other.type)*/, config(other.config), calibration(other.calibration), 
//     camera(other.camera), brightness(other.brightness), exposure(other.exposure), whitebalance(other.whitebalance) {}
// VisionCamera::VisionCamera(VisionCamera&& other) : 
//     VideoCamera(other.GetHandle())/*, type(other.type)*/, config(std::move(other.config)), calibration(std::move(other.calibration)), 
//     camera(std::move(other.camera)), brightness(other.brightness), exposure(other.exposure), whitebalance(other.whitebalance) {}
VisionCamera::~VisionCamera() {
    this->deleteEntries();
}

// VisionCamera& VisionCamera::operator=(const VisionCamera& other) {
//     this->m_handle = other.m_handle;
//     /*this->type = other.type;*/
//     this->config = other.config;
//     this->calibration = other.calibration;
//     this->camera = other.camera;
//     this->brightness = other.brightness;
//     this->exposure = other.exposure;
//     this->whitebalance = other.whitebalance;
// }
// VisionCamera& VisionCamera::operator=(VisionCamera&& other) {    // there's really no point because wpi::json doesn't have a move operator
//     this->m_handle = other.m_handle;
//     this->type = other.type;
//     this->config = std::move(other.config);
//     this->calibration = std::move(other.calibration);
//     this->camera = std::move(other.camera);
//     this->brightness = other.brightness;
//     this->exposure = other.exposure;
//     this->whitebalance = other.whitebalance;
// }

// cs::VideoSource::Kind VisionCamera::getType() const {
// 	return this->type;
// }
bool VisionCamera::isValidJson() const {
	return this->config.is_object();
}
const wpi::json& VisionCamera::getJson() const {
	return this->config;
}
bool VisionCamera::isValidStreamJson() const {
	return this->config.count("stream") > 0;
}
wpi::json VisionCamera::getStreamJson() const {
	if(this->config.count("stream") > 0) {
		return this->config.at("stream");
	}
	return wpi::json();
}

bool VisionCamera::getJsonCameraMatrix(cv::Mat_<double>& array) const {
    if(this->calibration.is_object()) {
        try{
            wpi::json matx = this->calibration.at("camera_matrix");
            for(size_t i = 0; i < 3; i++) {
                for(size_t j = 0; j < 3; j++) {
                    array[i][j] = matx.at(i).at(j).get<double>();
                }
            }
        } catch (const wpi::json::exception& e) {
            wpi::errs() << "Failed to parse camera matrix: " << e.what() << newline;
        }
        return true;
    }
    return false;
}
bool VisionCamera::getJsonCameraMatrix(cv::Mat_<float>& array) const {
    if(this->calibration.is_object()) {
        try{
            wpi::json matx = this->calibration.at("camera_matrix");
            for(size_t i = 0; i < 3; i++) {
                for(size_t j = 0; j < 3; j++) {
                    array[i][j] = matx.at(i).at(j).get<float>();
                }
            }
        } catch (const wpi::json::exception& e) {
            wpi::errs() << "Failed to parse camera matrix: " << e.what() << newline;
        }
        return true;
    }
    return false;
}
bool VisionCamera::getJsonDistortionCoefs(cv::Mat_<double>& array) const {
    if(this->calibration.is_object()) {
        try{
            wpi::json matx = this->calibration.at("distortion");
            for(size_t i = 0; i < 5; i++) {
                array[0][i] = matx.at(0).at(i).get<double>();
            }
        } catch (const wpi::json::exception& e) {
            wpi::errs() << "Failed to parse camera matrix: " << e.what() << newline;
        }
        return true;
    }
    return false;
}
bool VisionCamera::getJsonDistortionCoefs(cv::Mat_<float>& array) const {
    if(this->calibration.is_object()) {
        try{
            wpi::json matx = this->calibration.at("distortion");
            for(size_t i = 0; i < 5; i++) {
                array[0][i] = matx.at(0).at(i).get<float>();
            }
        } catch (const wpi::json::exception& e) {
            wpi::errs() << "Failed to parse camera matrix: " << e.what() << newline;
        }
        return true;
    }
    return false;
}

uint64_t VisionCamera::getFrame(cv::Mat& o_frame, double timeout) const {
	if(this->IsConnected()) {
		return this->raw.GrabFrame(o_frame, timeout);
	} else {
		o_frame = cv::Mat::zeros(this->getResolution(), CV_8UC3);
		return 0;
	}
}
uint64_t VisionCamera::getFrameNoTmO(cv::Mat& o_frame) const {
	if(this->IsConnected()) {
		return this->raw.GrabFrameNoTimeout(o_frame);
	} else {
		o_frame = cv::Mat::zeros(this->getResolution(), CV_8UC3);
		return 0;
	}
}

int VisionCamera::getWidth() const {
    return this->properties.width;
}
int VisionCamera::getHeight() const {
    return this->properties.height;
}
int VisionCamera::getPixels() const {
    cs::VideoMode mode = this->properties;
    return (mode.width*mode.height);
}
int VisionCamera::getConfigFPS() const {
    return this->properties.fps;
}
cv::Size VisionCamera::getResolution() const {
    return cv::Size(this->properties.width, properties.height);
}

int8_t VisionCamera::getBrightness() const {
    return this->brightness;
}
int8_t VisionCamera::getExposure() const {
    return this->exposure;
}
int16_t VisionCamera::getWhiteBalance() const {
    return this->whitebalance;
}

void VisionCamera::setBrightness(int8_t val) {
    this->brightness = (val > 100 ? 100 : (val < 0 ? 0 : val));
    this->SetBrightness(this->brightness);
}
void VisionCamera::setWhiteBalance(int16_t val) {
    val < 0 ? this->SetWhiteBalanceAuto() : this->SetWhiteBalanceManual(val);   // find bounds and add checking
    this->whitebalance = val;
}
void VisionCamera::setExposure(int8_t val) {
    val < 0 ? this->SetExposureAuto() : this->SetExposureManual(val > 100 ? 100 : val);
    this->exposure = (val > 100 ? 100 : val);
}

void VisionCamera::setNetworkBase(std::shared_ptr<nt::NetworkTable> table) {
    this->camera = table->GetSubTable("Cameras")->GetSubTable(this->GetName());
}
void VisionCamera::setNetworkAdjustable() {
    this->setBrightnessAdjustable();
    this->setWhiteBalanceAdjustable();
    this->setExposureAdjustable();
}
void VisionCamera::deleteEntries() {
    this->camera->Delete("Brightness");
    this->camera->Delete("WhiteBalance");
    this->camera->Delete("Exposure");
}

void VisionCamera::setBrightnessAdjustable() {
    this->camera->PutNumber("Brightness", this->brightness);
	this->camera->GetEntry("Brightness").AddListener(
		[this](const nt::EntryNotification& event){
			if(event.value->IsDouble()) {
				this->setBrightness(event.value->GetDouble());
			}
		},
		NT_NOTIFY_IMMEDIATE | NT_NOTIFY_NEW | NT_NOTIFY_UPDATE
	);
}
void VisionCamera::setWhiteBalanceAdjustable() {
    this->camera->PutNumber("WhiteBalance", this->whitebalance);
	this->camera->GetEntry("WhiteBalance").AddListener(
		[this](const nt::EntryNotification& event){
			if(event.value->IsDouble()) {
				this->setWhiteBalance(event.value->GetDouble());
			}
		},
		NT_NOTIFY_IMMEDIATE | NT_NOTIFY_NEW | NT_NOTIFY_UPDATE
	);
}
void VisionCamera::setExposureAdjustable() {
    this->camera->PutNumber("Exposure", this->exposure);
	this->camera->GetEntry("Exposure").AddListener(
		[this](const nt::EntryNotification& event){
			if(event.value->IsDouble()) {
				this->setExposure(event.value->GetDouble());
			}
		},
		NT_NOTIFY_IMMEDIATE | NT_NOTIFY_NEW | NT_NOTIFY_UPDATE
	);
}