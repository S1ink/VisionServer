#include "vision.h"

#include "cpp-tools/src/resources.h"


// cs::CvSink switchedCameraVision(const std::vector<VisionCamera>& cameras, std::shared_ptr<nt::NetworkTable> table) {
// 	if(!table->ContainsKey("Camera Index")) {
// 		table->PutNumber("Camera Index", 0);
// 	}
// 	if(!table->ContainsKey("Cameras Available")) {
// 		table->PutNumber("Cameras Available", cameras.size());
// 	}
// 	static cs::CvSink source;
// 	if(cameras.size() > 0) {
// 		source = cameras[0].getSink();
// 	}
// 	table->GetEntry("Camera Index").AddListener(
// 		[&cameras](const nt::EntryNotification& event) {
// 			if(event.value->IsDouble()) {
// 				size_t idx = event.value->GetDouble();
// 				if(idx >= 0 && idx < cameras.size()) {
// 					source.SetSource(cameras[idx]);
// 					source.SetConfigJson(cameras[idx].getStreamJson());
// 				}
// 			}
// 		},
// 		NT_NOTIFY_IMMEDIATE | NT_NOTIFY_NEW | NT_NOTIFY_UPDATE
// 	);
// 	return source;
// }

cs::VideoMode getJsonVideoMode(const wpi::json& config) {
    cs::VideoMode mode;

// the following is copied from: https://github.com/wpilibsuite/allwpilib/blob/main/cscore/src/main/native/cpp/SourceImpl.cpp#L184
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

// void addNetTableVar(bool& var, const std::string& name, std::shared_ptr<nt::NetworkTable> table) {
// 	if(!table->ContainsKey(name)) {
// 		table->PutBoolean(name, var);
// 	} else {}
// 	table->GetEntry(name).AddListener(
// 		[&var](const nt::EntryNotification& event){
// 			if(event.value->IsBoolean()) {
// 				var = event.value->GetBoolean();
// 				//std::cout << " Networktable var(bool) updated to : " << var << newline;
// 			}
// 		},
// 		NT_NOTIFY_IMMEDIATE | NT_NOTIFY_NEW | NT_NOTIFY_UPDATE
// 	);
// }

// const std::array<std::array<cv::Scalar, 3>, 3> markup_map = {
//     std::array<cv::Scalar, 3>{cv::Scalar(255, 0, 0), cv::Scalar(255, 127, 0), cv::Scalar(255, 255, 0)},	//blue
// 	std::array<cv::Scalar, 3>{cv::Scalar(0, 255, 0), cv::Scalar(0, 255, 127), cv::Scalar(0, 255, 255)},	//green
// 	std::array<cv::Scalar, 3>{cv::Scalar(0, 0, 255), cv::Scalar(127, 0, 255), cv::Scalar(255, 0, 255)},	//red
// };