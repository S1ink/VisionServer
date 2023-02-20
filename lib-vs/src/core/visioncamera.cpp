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
VisionCamera::VisionCamera(VisionCamera&& other) : 
    VideoCamera(other.GetHandle()),
    config(std::move(other.config)),
    calibration(std::move(other.calibration)),
    raw(std::move(other.raw)),
    camera_matrix(std::move(other.camera_matrix)),
    distortion(std::move(other.distortion)),
    properties(other.properties),
    ntable(other.ntable),
    nt_brightness(std::move(other.nt_brightness)),
    nt_exposure(std::move(other.nt_exposure)),
    nt_whitebalance(std::move(other.nt_whitebalance)),
    listener_handle(other.listener_handle)
{
    other.m_handle = 0;
    other.listener_handle = 0;
}
VisionCamera::~VisionCamera() {
    this->ntable->RemoveListener(this->listener_handle);
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
VisionCamera& VisionCamera::operator=(VisionCamera&& other) {
    this->m_handle = other.m_handle;
    other.m_handle = 0;
    this->config = std::move(other.config);
    this->calibration = std::move(other.calibration);
    this->raw = std::move(other.raw);
    this->camera_matrix = std::move(other.camera_matrix);
    this->distortion = std::move(other.distortion);
    this->properties = std::move(other.properties);
    this->ntable = other.ntable;
    this->nt_brightness = std::move(other.nt_brightness);
    this->nt_whitebalance = std::move(other.nt_whitebalance);
    this->nt_exposure = std::move(other.nt_exposure);
    this->listener_handle = other.listener_handle;
    other.listener_handle = 0;
    return *this;
}

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

bool VisionCamera::setCalibrationJson(const wpi::json& j) {
    if(!j.is_object()) {
        return false;
    }
    this->calibration = j;
    return
        this->getJsonCameraMatrix(this->camera_matrix) &&
        this->getJsonDistortionCoefs(this->distortion);
}
bool VisionCamera::setCameraMatrix(const cv::Mat_<float>& mat) {
    if(mat.size().area() != default_matrix.size().area()) {
        return false;
    }
    this->camera_matrix = mat.reshape(0, 3);
    return true;
}
bool VisionCamera::setDistortionCoefs(const cv::Mat_<float>& mat) {
    if(mat.size().area() != default_distort.size().area()) {
        return false;
    }
    this->distortion = mat.reshape(0, 1);
    return true;
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
    return cv::Size(this->properties.width, this->properties.height);
}

int VisionCamera::getBrightness() const {
    return this->nt_brightness.Get();
}
int VisionCamera::getExposure() const {
    return this->nt_exposure.Get();
}
int VisionCamera::getWhiteBalance() const {
    return this->nt_whitebalance.Get();
}

void VisionCamera::setBrightness(int val) {
    this->nt_brightness.Set(this->_setBrightness(val));
}
void VisionCamera::setWhiteBalance(int val) {
    this->nt_whitebalance.Set(this->_setWhiteBalance(val));
}
void VisionCamera::setExposure(int val) {
    this->nt_exposure.Set(this->_setExposure(val));
}

void VisionCamera::setNetworkBase(const std::shared_ptr<nt::NetworkTable>& table) {
    this->ntable = table->GetSubTable("Cameras")->GetSubTable(this->GetName());
}
void VisionCamera::setNetworkAdjustable() {
    this->nt_brightness = this->ntable->GetIntegerTopic("Brightness").GetEntry(50);
    this->nt_whitebalance = this->ntable->GetIntegerTopic("WhiteBalance").GetEntry(-1);
    this->nt_exposure = this->ntable->GetIntegerTopic("Exposure").GetEntry(-1);
    this->setBrightness(50);
    this->setWhiteBalance(-1);
    this->setExposure(-1);
    nt::NetworkTableInstance::GetDefault().RemoveListener(this->listener_handle);
    this->listener_handle = this->ntable->AddListener(
        nt::EventFlags::kValueRemote | nt::EventFlags::kTopic,
        [this](nt::NetworkTable *table, std::string_view key, const nt::Event& e) {
            if(const nt::ValueEventData* v = e.GetValueEventData()) {
                NT_Handle h = v->topic;
                int val = (v->value.IsInteger() ?
                    v->value.GetInteger() : (v->value.IsDouble() ?
                        v->value.GetDouble() : (v->value.IsFloat() ?
                            v->value.GetFloat() : 0xFFFFFFFF) ) );
                if(val != 0xFFFFFFFF) {
                    if(h == this->nt_brightness.GetTopic().GetHandle()) {
                        this->_setBrightness(val);
                    } else
                    if(h == this->nt_exposure.GetTopic().GetHandle()) {
                        this->_setExposure(val);
                    } else
                    if(h == this->nt_whitebalance.GetTopic().GetHandle()) {
                        this->_setWhiteBalance(val);
                    }
                }
            }
        }
    );
}

int VisionCamera::_setBrightness(int val) {
    val = (val > 100 ? 100 : (val < 0 ? 0 : val));
    this->SetBrightness(val);
    return val;
}
int VisionCamera::_setWhiteBalance(int val) {
    val < 0 ? this->SetWhiteBalanceAuto() : this->SetWhiteBalanceManual(val);
    return val;
}
int VisionCamera::_setExposure(int val) {
    val = (val > 100 ? 100 : val);
    val < 0 ? this->SetExposureAuto() : this->SetExposureManual(val);
    return val;
}