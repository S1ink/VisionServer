#include "visioncamera.h"

VisionCamera::VisionCamera(CS_Source handle) : 
	VideoCamera(handle), type((cs::VideoSource::Kind)CS_GetSourceKind(handle, &this->m_status))
{}
VisionCamera::VisionCamera(const cs::VideoSource& source, const wpi::json& config) : 
	VideoCamera(source.GetHandle()), type((cs::VideoSource::Kind)CS_GetSourceKind(source.GetHandle(), nullptr)), config(config)
{}
VisionCamera::VisionCamera(const cs::UsbCamera& source, const wpi::json& config) : 
	VideoCamera(source.GetHandle()), type(cs::VideoSource::Kind::kUsb), config(config)
{}
VisionCamera::VisionCamera(const cs::HttpCamera& source, const wpi::json& config) :
	VideoCamera(source.GetHandle()), type(cs::VideoSource::Kind::kHttp), config(config)
{}
VisionCamera::VisionCamera(const wpi::json& source_config) :	// add and/or find a way to determine device type from config json
	type(cs::VideoSource::Kind::kUsb), config(source_config)
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
}

cs::VideoSource::Kind VisionCamera::getType() const {
	return this->type;
}
bool VisionCamera::isValidJson() const {
	return this->config.is_object();
}
wpi::json VisionCamera::getJson() const {
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

cs::CvSink VisionCamera::getVideo() const {
	cs::CvSink video = frc::CameraServer::GetInstance()->GetVideo(*this);
	if(this->config.count("stream") > 0) {
		video.SetConfigJson(this->config.at("stream"));
	}
	return video;
}
cs::CvSource VisionCamera::getSeparateServer() const {
	return frc::CameraServer::GetInstance()->PutVideo(("Camera stream: " + this->GetName()), this->getWidth(), this->getHeight());
}

int VisionCamera::getWidth() const {
    return this->GetVideoMode().width;
}
int VisionCamera::getHeight() const {
    return this->GetVideoMode().height;
}
int VisionCamera::getPixels() const {
    cs::VideoMode mode = this->GetVideoMode();
    return (mode.width*mode.height);
}
int VisionCamera::getSetFPS() const {
    return this->GetVideoMode().fps;
}
cv::Size VisionCamera::getResolution() const {
    return cv::Size(this->GetVideoMode().height, this->GetVideoMode().width);
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

void VisionCamera::setBrightnessAdjustable() {
	this->setBrightnessAdjustable(nt::NetworkTableInstance::GetDefault().GetTable(this->GetName()));
}
void VisionCamera::setBrightnessAdjustable(std::shared_ptr<nt::NetworkTable> table) {
    const char* name = "Brightness";
    if(!table->ContainsKey(name)) {
		table->PutNumber(name, this->brightness);
	} else {}
	table->GetEntry(name).AddListener(
		[this](const nt::EntryNotification& event){
			if(event.value->IsDouble()) {
				this->setBrightness(event.value->GetDouble());
			}
		},
		NT_NOTIFY_IMMEDIATE | NT_NOTIFY_NEW | NT_NOTIFY_UPDATE
	);
}
void VisionCamera::setWhiteBalanceAdjustable() {
	this->setWhiteBalanceAdjustable(nt::NetworkTableInstance::GetDefault().GetTable(this->GetName()));
}
void VisionCamera::setWhiteBalanceAdjustable(std::shared_ptr<nt::NetworkTable> table) {
    const char* name = "WhiteBalance";
    if(!table->ContainsKey(name)) {
		table->PutNumber(name, this->whitebalance);
	} else {}
	table->GetEntry(name).AddListener(
		[this](const nt::EntryNotification& event){
			if(event.value->IsDouble()) {
				this->setWhiteBalance(event.value->GetDouble());
			}
		},
		NT_NOTIFY_IMMEDIATE | NT_NOTIFY_NEW | NT_NOTIFY_UPDATE
	);
}
void VisionCamera::setExposureAdjustable() {
	this->setExposureAdjustable(nt::NetworkTableInstance::GetDefault().GetTable(this->GetName()));
}
void VisionCamera::setExposureAdjustable(std::shared_ptr<nt::NetworkTable> table) {
    const char* name = "Exposure";
    if(!table->ContainsKey(name)) {
		table->PutNumber(name, this->exposure);
	} else {}
	table->GetEntry(name).AddListener(
		[this](const nt::EntryNotification& event){
			if(event.value->IsDouble()) {
				this->setExposure(event.value->GetDouble());
			}
		},
		NT_NOTIFY_IMMEDIATE | NT_NOTIFY_NEW | NT_NOTIFY_UPDATE
	);
}