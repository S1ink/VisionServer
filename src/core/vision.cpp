#include "vision.h"

#include <wpi/raw_ostream.h>
#include <wpi/raw_istream.h>


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
        wpi::errs() << "Config error in " << file << /*": byte " << (int)e.byte <<*/ ": " << e.what() << newline;
        return false;
    }
    if (!j.is_object()) {
        wpi::errs() << "Config error in " << file << ": must be JSON object\n";
        return false;
    }

    if(j.count("ntmode") != 0) {
        try {
            std::string str = j.at("ntmode").get<std::string>();
            if(wpi::equals_lower(str, "client")) {
                wpi::outs() << "Setting up NetworkTables in CLIENT mode\n";
                try { 
                    nt::NetworkTableInstance::GetDefault().StartClientTeam(j.at("team").get<unsigned int>());
                    nt::NetworkTableInstance::GetDefault().StartDSClient();
                }
                catch (const wpi::json::exception& e) {
                    wpi::errs() << "Config error in " << file << ": could not read team number: " << e.what() << newline;
                    return false;
                }
            } else if (wpi::equals_lower(str, "server")) {
                wpi::outs() << "Setting up NetworkTables in SERVER mode\n";
                nt::NetworkTableInstance::GetDefault().StartServer();
            } else {
                wpi::outs() << "Setting up NetworkTables for SIMULATION mode - host: " << str << newline;
                nt::NetworkTableInstance::GetDefault().StartClient(str.c_str());
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
						//std::cout << "MAKING CAMERA:\n" << camera.dump() << newline;
                    } catch (const wpi::json::exception& e) {
                        wpi::errs() << "Config error in " << file << ": failed to get configuration object: " << e.what() << newline;  // print out more info if needed
                        cameras.emplace_back(camera);
                    }
                } else {
                    cameras.emplace_back(camera);
                }
#ifdef REMOVE_DISCONNECTED_CAMERAS
                if(!cameras.back().IsConnected()) { cameras.pop_back(); }
#endif
            }
        }
        catch (const wpi::json::exception& e) {
            wpi::errs() << "Config error in " << file << ": could not read cameras: " << e.what() << newline;
        }
    }
    // if(j.count("switched cameras") != 0) {
    //     try {
    //         for(const wpi::json::value_type& stream : j.at("switched cameras")) {
    //             cs::MjpegServer server;
    //             try { server = frc::CameraServer::GetInstance()->AddSwitchedCamera(stream.at("name").get<std::string>()); }
    //             catch (const wpi::json::exception& e) {
    //                 wpi::errs() << "Could not read switched camera name: " << e.what() << newline;
    //                 return false;
    //             }
    //             try { nt::NetworkTableInstance::GetDefault()
    //                 .GetEntry(stream.at("key").get<std::string>())
    //                 .AddListener(
    //                     [server, cameras](const nt::EntryNotification& event) mutable {
    //                         if(event.value->IsDouble()) {
    //                             size_t i = event.value->GetDouble();
    //                             if(i >= 0 && i < cameras.size()) {
    //                                 server.SetSource(cameras[i]);
    //                             }
    //                         } else if (event.value->IsString()) {
    //                             wpi::StringRef str = event.value->GetString();
    //                             for(size_t i = 0; i < cameras.size(); i++) {
    //                                 if(str == cameras[i].GetName()) {
    //                                     server.SetSource(cameras[i]);
    //                                     break;
    //                                 }
    //                             }
    //                         }
    //                     },
    //                     NT_NOTIFY_IMMEDIATE | NT_NOTIFY_NEW | NT_NOTIFY_UPDATE
    //                 );
    //             }
    //             catch (const wpi::json::exception& e) {
    //                 wpi::errs() << "Config error in " << file << ": could not read key: " << e.what() << newline;
    //                 return false;
    //             }
    //         }
    //     }
    //     catch (const wpi::json::exception& e) {
    //         wpi::errs() << "Config error in " << file << ": could not read switched cameras: " << e.what() << newline;
    //         return false;
    //     }
    // }

    return true;
}

cs::CvSink switchedCameraVision(const std::vector<VisionCamera>& cameras, std::shared_ptr<nt::NetworkTable> table) {
	if(!table->ContainsKey("Camera Index")) {
		table->PutNumber("Camera Index", 0);
	}
	if(!table->ContainsKey("Cameras Available")) {
		table->PutNumber("Cameras Available", cameras.size());
	}
	static cs::CvSink source;
	if(cameras.size() > 0) {
		source = cameras[0].getSink();
	}
	table->GetEntry("Camera Index").AddListener(
		[&cameras](const nt::EntryNotification& event) {
			if(event.value->IsDouble()) {
				size_t idx = event.value->GetDouble();
				if(idx >= 0 && idx < cameras.size()) {
					source.SetSource(cameras[idx]);
					source.SetConfigJson(cameras[idx].getStreamJson());
				}
			}
		},
		NT_NOTIFY_IMMEDIATE | NT_NOTIFY_NEW | NT_NOTIFY_UPDATE
	);
	return source;
}

cs::VideoMode getJsonVideoMode(const wpi::json& config) {
    cs::VideoMode mode;

// the following is copied from: https://github.com/wpilibsuite/allwpilib/blob/main/cscore/src/main/native/cpp/SourceImpl.cpp (184)
    if (config.count("pixel format") != 0) {
        try {
            std::string str = config.at("pixel format").get<std::string>();
            if (wpi::equals_lower(str, "mjpeg")) {
                mode.pixelFormat = cs::VideoMode::kMJPEG;
            } else if (wpi::equals_lower(str, "yuyv")) {
                mode.pixelFormat = cs::VideoMode::kYUYV;
            } else if (wpi::equals_lower(str, "rgb565")) {
                mode.pixelFormat = cs::VideoMode::kRGB565;
            } else if (wpi::equals_lower(str, "bgr")) {
                mode.pixelFormat = cs::VideoMode::kBGR;
            } else if (wpi::equals_lower(str, "gray")) {
                mode.pixelFormat = cs::VideoMode::kGray;
            } else {
                mode.pixelFormat = cs::VideoMode::kUnknown;
            }
        } catch (const wpi::json::exception& e) {
            mode.pixelFormat = cs::VideoMode::kUnknown;
        }
    }
    if (config.count("width") != 0) {
        try {
            mode.width = config.at("width").get<unsigned int>();
        } catch (const wpi::json::exception& e) {
            mode.width = 0;
        }
    }
    if (config.count("height") != 0) {
        try {
            mode.height = config.at("height").get<unsigned int>();
        } catch (const wpi::json::exception& e) {}
    }
    if (config.count("fps") != 0) {
        try {
            mode.fps = config.at("fps").get<unsigned int>();
        } catch (const wpi::json::exception& e) {}
    }
    return mode;
}

void addNetTableVar(bool& var, const std::string& name, std::shared_ptr<nt::NetworkTable> table) {
	if(!table->ContainsKey(name)) {
		table->PutBoolean(name, var);
	} else {}
	table->GetEntry(name).AddListener(
		[&var](const nt::EntryNotification& event){
			if(event.value->IsBoolean()) {
				var = event.value->GetBoolean();
				//std::cout << " Networktable var(bool) updated to : " << var << newline;
			}
		},
		NT_NOTIFY_IMMEDIATE | NT_NOTIFY_NEW | NT_NOTIFY_UPDATE
	);
}

// const std::array<std::array<cv::Scalar, 3>, 3> markup_map = {
//     std::array<cv::Scalar, 3>{cv::Scalar(255, 0, 0), cv::Scalar(255, 127, 0), cv::Scalar(255, 255, 0)},	//blue
// 	std::array<cv::Scalar, 3>{cv::Scalar(0, 255, 0), cv::Scalar(0, 255, 127), cv::Scalar(0, 255, 255)},	//green
// 	std::array<cv::Scalar, 3>{cv::Scalar(0, 0, 255), cv::Scalar(127, 0, 255), cv::Scalar(255, 0, 255)},	//red
// };