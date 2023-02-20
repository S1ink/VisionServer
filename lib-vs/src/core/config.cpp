#include "config.h"

#include <wpi/raw_ostream.h>
#include <wpi/raw_istream.h>
#include <wpi/StringExtras.h>
#include <networktables/NetworkTableInstance.h>

#include "cpp-tools/src/resources.h"


bool loadJson(wpi::json& j, const char* file) {
	std::error_code ec;
    wpi::raw_fd_istream is(file, ec);
    if (ec) {
        wpi::errs() << "Could not open '" << file << "': " << ec.message() << newline;
        return false;
    }
    try { j = wpi::json::parse(is); }
    catch (const wpi::json::parse_error& e) {
        wpi::errs() << "Config error in " << file << /*": byte " << (int)e.byte <<*/ ": " << e.what() << newline;
        return false;
    }
    if (!j.is_object()) {
        wpi::errs() << "Config error in " << file << ": must be JSON object\n";
        return false;
    }
	wpi::errs().flush();
	return true;
}
bool initNT(const char* file) {

	wpi::json j;
	if(!loadJson(j, file)) { return false; }

    if(j.count("ntmode") != 0) {
        try {
            std::string str = j.at("ntmode").get<std::string>();
            if(wpi::equals_lower(str, "client")) {
                wpi::outs() << "Setting up NetworkTables in CLIENT mode\n";
                try {
#if NT_CLIENT_VERSION == 4
                    nt::NetworkTableInstance::GetDefault().StartClient4("VisionServer");
#else
                    nt::NetworkTableInstance::GetDefault().StartClient3("VisionServer");
#endif
                    nt::NetworkTableInstance::GetDefault().SetServerTeam(j.at("team").get<unsigned int>());
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
#if NT_CLIENT_VERSION == 4
                nt::NetworkTableInstance::GetDefault().StartClient4("VisionServer");
#else
            	nt::NetworkTableInstance::GetDefault().StartClient3("VisionServer");
#endif
                nt::NetworkTableInstance::GetDefault().SetServer(str.c_str());
            }
        } catch (const wpi::json::exception& e) {
            wpi::errs() << "Config error in " << file << ": coud not read ntmode: " << e.what() << newline;
        }
    }
	wpi::outs().flush();
    return true;

}
bool initSimNT(const char* sim_ip) {

    wpi::outs() << "Setting up NetworkTables for SIMULATION mode - host: " << sim_ip << newline;
#if NT_CLIENT_VERSION == 4
    nt::NetworkTableInstance::GetDefault().StartClient4("VisionServer");
#else
    nt::NetworkTableInstance::GetDefault().StartClient3("VisionServer");
#endif
    nt::NetworkTableInstance::GetDefault().SetServer(sim_ip);
    wpi::outs().flush();
    return true;

}
bool createCameras(std::vector<VisionCamera>& cameras, const char* file) {
	
	wpi::json j;
	if(!loadJson(j, file)) { return false; }

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
	wpi::errs().flush();
	return true;

}
bool createCameras(std::vector<VisionCamera>& cameras, CalibList calibrations, const char* file) {

	wpi::json j;
	if(!loadJson(j, file)) { return false; }

	if(j.count("cameras") < 1) {
        wpi::errs() << "Config error in " << file << ": no camera configs found, this program requires cameras to function\n";
    } else {
        try {
			bool has_calibs = j.count("calibrations") > 0;
            for(const wpi::json& camera : j.at("cameras")) {
				cameras.emplace_back(camera);
                if(camera.count("properties") && camera.at("properties").count("type")) {
                    try {
                        std::string type = camera.at("properties").at("type").get<std::string>();
                        using SizeItem = std::pair<cv::Size2i, std::array<cv::Mat_<float>, 2> >;
						using TypeItem = std::pair<const char*, std::vector<SizeItem> >;
                        cv::Size2i sz = cameras.back().getResolution();
                        for(const TypeItem& item : calibrations) {
							if(wpi::equals_lower(type, item.first)) {
                                for(const SizeItem& arr : item.second) {
                                    if(sz == arr.first) {
                                        cameras.back().setCameraMatrix(arr.second[0]);
                                        cameras.back().setDistortionCoefs(arr.second[1]);
                                        break;
                                    }
                                }
							}
						}
                    } catch (const wpi::json::exception& e) {
                        wpi::errs() << "Config error in " << file << ": failed to get 'type' property: " << e.what() << newline;  // print out more info if needed
                    }
                } else if(has_calibs && camera.count("calibration") > 0) {
                    try {
                        wpi::json calibration = j.at("calibrations").at(camera.at("calibration").get<std::string>());
                        cameras.back().setCalibrationJson(calibration);
                    } catch (const wpi::json::exception& e) {
                        wpi::errs() << "Config error in " << file << ": failed to get configuration object: " << e.what() << newline;  // print out more info if needed
                    }
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
	wpi::errs().flush();
	return true;

}



bool readConfig(std::vector<VisionCamera>& cameras, const char* file) {

	wpi::json j;
	if(!loadJson(j, file)) { return false; }

    if(j.count("ntmode") != 0) {
        try {
            std::string str = j.at("ntmode").get<std::string>();
            if(wpi::equals_lower(str, "client")) {
                wpi::outs() << "Setting up NetworkTables in CLIENT mode\n";
                try {
#if NT_CLIENT_VERSION == 4
                    nt::NetworkTableInstance::GetDefault().StartClient4("VisionServer");
#else
                    nt::NetworkTableInstance::GetDefault().StartClient3("VisionServer");
#endif
                    nt::NetworkTableInstance::GetDefault().SetServerTeam(j.at("team").get<unsigned int>());
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
#if NT_CLIENT_VERSION == 4
                    nt::NetworkTableInstance::GetDefault().StartClient4("VisionServer");
#else
                    nt::NetworkTableInstance::GetDefault().StartClient3("VisionServer");
#endif
                nt::NetworkTableInstance::GetDefault().SetServer(str.c_str());
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

    wpi::outs().flush();
    return true;
}