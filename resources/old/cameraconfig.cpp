#include "cameraconfig.h"

wpi::raw_ostream& parseError(const char* path) {
	return wpi::errs() << "config error in '" << path << "': ";
}



bool CameraConfig::read(const wpi::json& source, const char* file) {

    try { this->name = source.at("name").get<std::string>(); }
    catch (const wpi::json::exception& e) {
        parseError(file) << "could not read camera name: " << e.what() << newline;
        return false;
    }
    try { this->path = source.at("path").get<std::string>(); }
    catch (const wpi::json::exception& e) {
        parseError(file) << "camera '" << this->name
            << "': could not read path: " << e.what() << newline;
        return false;
    }
    if (source.count("stream") != 0) {
        this->stream_config = source.at("stream");
    }
    this->config = source;
    this->camera = std::move(cs::UsbCamera(this->name, this->path));

    this->camera.SetConfigJson(this->config);
    this->camera.SetConnectionStrategy(cs::VideoSource::kConnectionKeepOpen);

    return true;
}
cs::MjpegServer CameraConfig::start() const {

    wpi::outs() << "Starting camera '" << this->name << "' on " << this->path << newline;
    frc::CameraServer* inst = frc::CameraServer::GetInstance();
    cs::MjpegServer server = inst->StartAutomaticCapture(this->camera);

    if (this->stream_config.is_object())
        server.SetConfigJson(this->stream_config);

    return server;

}
bool CameraConfig::start(const wpi::json& source, const char* file) {
    bool ret = read(source, file);
    start();
    return ret;
}
CameraConfig::CameraConfig(const wpi::json& source, const char* file) {
    this->read(source, file);
}



bool SwitchedCameraConfig::read(const wpi::json& source, const char* file) {

    try { this->name = source.at("name").get<std::string>(); }
    catch (const wpi::json::exception& e) {
        parseError(file) << "could not read switched camera name: " << e.what() << newline;
        return false;
    }
    try { this->key = source.at("key").get<std::string>(); }
    catch (const wpi::json::exception& e) {
        parseError(file) << "switched camera '" << this->name
            << "': could not read key: " << e.what() << '\n';
        return false;
    }

    return true;
}
cs::MjpegServer SwitchedCameraConfig::start(const StreamConfig& data) const {

    wpi::outs() << "Starting switched camera '" << this->name << "' on " << this->key << '\n';
    cs::MjpegServer server = frc::CameraServer::GetInstance()->AddSwitchedCamera(this->name);

    nt::NetworkTableInstance::GetDefault()
        .GetEntry(this->key)
        .AddListener(
            [data, server](const nt::EntryNotification& event) mutable {
                if (event.value->IsDouble()) {
                    int i = event.value->GetDouble();
                    if (i >= 0 && i < data.cameras.size()) server.SetSource(data.cameras[i].camera);
                }
                else if (event.value->IsString()) {
                    wpi::StringRef str = event.value->GetString();
                    for (int i = 0; i < data.cameras.size(); ++i) {
                        if (str == data.cameras[i].name) {
                            server.SetSource(data.cameras[i].camera);
                            break;
                        }
                    }
                }
            },
            NT_NOTIFY_IMMEDIATE | NT_NOTIFY_NEW | NT_NOTIFY_UPDATE);

    return server;
}
SwitchedCameraConfig::SwitchedCameraConfig(const wpi::json& source, const char* file) {
    this->read(source, file);
}



bool StreamConfig::parse(const char* file) {

    std::error_code ec;
    wpi::raw_fd_istream is(file, ec);
    if (ec) {
        wpi::errs() << "could not open '" << file << "': " << ec.message() << newline;
        return false;
    }

    wpi::json j;
    try { j = wpi::json::parse(is); }
    catch (const wpi::json::parse_error& e) {
        parseError(file) << "byte " << e.byte << ": " << e.what() << newline;
        return false;
    }
    if (!j.is_object()) {
        parseError(file) << "must be JSON object\n";
        return false;
    }

    try { team = j.at("team").get<unsigned int>(); }
    catch (const wpi::json::exception& e) {
        parseError(file) << "could not read team number: " << e.what() << newline;
        return false;
    }
    if (j.count("ntmode") != 0) {
        try {
            std::string str = j.at("ntmode").get<std::string>();
            wpi::StringRef s(str);
            if (s.equals_lower("client")) { this->is_server = false; }
            else if (s.equals_lower("server")) { this->is_server = true; }
            else { parseError(file) << "could not understand ntmode value '" << str << "'\n"; }
        }
        catch (const wpi::json::exception& e) {
            parseError(file) << "could not read ntmode: " << e.what() << newline;
        }
    }

    try {
        for (const wpi::json::value_type& camera : j.at("cameras")) {
            wpi::outs() << "Reading camera: " << camera << newline; 
            for (size_t i = 0; i < this->cameras.size(); i++) {     // for some reason the json libraries iterate some blocks twice, thus making two cameras with the same properties
                if (camera == this->cameras[i].config) {
                    wpi::outs() << "Duplicate camera entry found!\n";
                    goto duplicate_found;
                }
            }
            this->cameras.emplace_back(CameraConfig());
            if (!this->cameras.back().read(camera, file)) { return false; }
            duplicate_found:;
        }
    }
    catch (const wpi::json::exception& e) {
        parseError(file) << "could not read cameras: " << e.what() << newline;
        return false;
    }
    if (j.count("switched cameras") != 0) {
        try {
            for (const wpi::json::value_type& camera : j.at("switched cameras")) {
                this->servers.emplace_back(SwitchedCameraConfig());
                if (!this->servers.back().read(camera, file)) { return false; }
            }
        }
        catch (const wpi::json::exception& e) {
            parseError(file) << "could not read switched cameras: " << e.what() << newline;
            return false;
        }
    }

    return true;
}
nt::NetworkTableInstance StreamConfig::setup() {

    nt::NetworkTableInstance ntinst = nt::NetworkTableInstance::GetDefault();

    if (this->is_server) {
        wpi::outs() << "Setting up NetworkTables server\n";
        ntinst.StartServer();
    }
    else {
        wpi::outs() << "Setting up NetworkTables client for team " << this->team << newline;
        ntinst.StartClientTeam(this->team);
        ntinst.StartDSClient();
    }

    for (const CameraConfig& config : this->cameras) {
        config.start();
    }
    for (const SwitchedCameraConfig& config : this->servers) {
        config.start(*this);
    }

    return ntinst;
}
StreamConfig::StreamConfig(const char* file) {
    this->parse(file);
}
