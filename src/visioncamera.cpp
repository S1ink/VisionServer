#include "visioncamera.h"

bool readConfig(std::vector<VisionCamera>& cameras, const char* file) {

	std::error_code ec;
    wpi::raw_fd_istream is(file, ec);
    if (ec) {
        wpi::errs() << "Could not open '" << file << "': " << ec.message() << newline;
        return false;
    }

    wpi::json j;
    try { j = wpi::json::parse(is); }
    catch (const wpi::json::parse_error& e) {
        wpi::errs() << "Config error in " << file << ": byte " << e.byte << ": " << e.what() << newline;
        return false;
    }
    if (!j.is_object()) {
        wpi::errs() << "Config error in " << file << ": must be JSON object\n";
        return false;
    }

    if(j.count("ntmode") != 0) {
        try {
            std::string str = j.at("ntmode").get<std::string>();
            wpi::StringRef s(str);
            if(s.equals_lower("client")) {
                wpi::outs() << "Setting up NetworkTables in CLIENT mode\n";
                try { 
                    nt::NetworkTableInstance::GetDefault().StartClientTeam(j.at("team").get<unsigned int>());
                    nt::NetworkTableInstance::GetDefault().StartDSClient();
                }
                catch (const wpi::json::exception& e) {
                    wpi::errs() << "Config error in " << file << ": could not read team number: " << e.what() << newline;
                    return false;
                }
            } else if (s.equals_lower("server")) {
                wpi::outs() << "Setting up NetworkTables in SERVER mode\n";
                nt::NetworkTableInstance::GetDefault().StartServer();
            } else {
                wpi::errs() << "Config error in " << file << ": could not understand ntmode value '" << str << "'\n";
            }
        } catch (const wpi::json::exception& e) {
            wpi::errs() << "Config error in " << file << ": coud not read ntmode: " << e.what() << newline;
        }
    }

    if(j.count("cameras") < 1) {
        wpi::errs() << "Config error in " << file << ": no camera configs found, this program requires cameras to function\n";
    } else {
        try {
            for(const wpi::json& camera : j.at("cameras")) {
                if(camera.count("calibration") > 0 && j.count("calibrations") > 0) {
                    wpi::json calibration;
                    try {
                        calibration = j.at("calibrations").at(camera.at("calibration").get<std::string>()); 
                        cameras.emplace_back(camera, calibration);
                    } catch (const wpi::json::exception& e) {
                        wpi::errs() << "Config error in " << file << ": failed to get configuration object: " << e.what() << newline;  // print out more info if needed
                        cameras.emplace_back(camera);
                    }
                } else {
                    cameras.emplace_back(camera);
                }
            }
        }
        catch (const wpi::json::exception& e) {
            wpi::errs() << "Config error in " << file << ": could not read cameras: " << e.what() << newline;
        }
    }
    if(j.count("switched cameras") != 0) {
#ifdef SWITCHED_CAMERAS_CONFIG
        try {
            for(const wpi::json::value_type& stream : j.at("switched cameras")) {
                cs::MjpegServer server;
                try { server = frc::CameraServer::GetInstance()->AddSwitchedCamera(stream.at("name").get<std::string>()); }
                catch (const wpi::json::exception& e) {
                    wpi::errs() << "Could not read switched camera name: " << e.what() << newline;
                    return false;
                }
                try { nt::NetworkTableInstance::GetDefault()
                    .GetEntry(stream.at("key").get<std::string>())
                    .AddListener(
                        [server, cameras](const nt::EntryNotification& event) mutable {
                            if(event.value->IsDouble()) {
                                size_t i = event.value->GetDouble();
                                if(i >= 0 && i < cameras.size()) {
                                    server.SetSource(cameras[i]);
                                }
                            } else if (event.value->IsString()) {
                                wpi::StringRef str = event.value->GetString();
                                for(size_t i = 0; i < cameras.size(); i++) {
                                    if(str == cameras[i].GetName()) {
                                        server.SetSource(cameras[i]);
                                        break;
                                    }
                                }
                            }
                        },
                        NT_NOTIFY_IMMEDIATE | NT_NOTIFY_NEW | NT_NOTIFY_UPDATE
                    );
                }
                catch (const wpi::json::exception& e) {
                    wpi::errs() << "Config error in " << file << ": could not read key: " << e.what() << newline;
                    return false;
                }
            }
        }
        catch (const wpi::json::exception& e) {
            wpi::errs() << "Config error in " << file << ": could not read switched cameras: " << e.what() << newline;
            return false;
        }
#else
		std::cout << "Switched cameras are ignored from config in this build - all cameras are already added to the vision processing server\n";
#endif
    }

    return true;
}

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
VisionCamera::VisionCamera(const wpi::json& source_config, const wpi::json& calibration) :	// add and/or find a way to determine device type from config json
	type(cs::VideoSource::Kind::kUsb), config(source_config), calibration(calibration)
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

bool VisionCamera::getCameraMatrix(cv::Mat_<double>& array) {
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
bool VisionCamera::getCameraMatrix(cv::Mat_<float>& array) {
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
bool VisionCamera::getDistortion(cv::Mat_<double>& array) {
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
bool VisionCamera::getDistortion(cv::Mat_<float>& array) {
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

void VisionCamera::setNetworkAdjustable() {
    this->setBrightnessAdjustable();
    this->setWhiteBalanceAdjustable();
    this->setExposureAdjustable();
}