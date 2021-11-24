#include "visioncamera.h"

VisionCamera::VisionCamera(const cs::VideoSource& source) : cs::VideoCamera(source.GetHandle()), brightness(this->GetBrightness()) {}

int VisionCamera::getWidth() {
    return this->GetVideoMode().width;
}
int VisionCamera::getHeight() {
    return this->GetVideoMode().height;
}
int VisionCamera::getPixels() {
    cs::VideoMode mode = this->GetVideoMode();
    return (mode.width*mode.height);
}
int VisionCamera::getSetFPS() {
    return this->GetVideoMode().fps;
}
cv::Size VisionCamera::getResolution() {
    return cv::Size(this->GetVideoMode().height, this->GetVideoMode().width);
}

int8_t VisionCamera::getBrightness() {
    return this->brightness;
}
int8_t VisionCamera::getExposure() {
    return this->exposure;
}
int16_t VisionCamera::getWhiteBalance() {
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