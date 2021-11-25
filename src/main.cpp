#include <networktables/NetworkTableInstance.h>
#include <networktables/EntryListenerFlags.h>
#include <networktables/NetworkTableEntry.h>
#include <networktables/NetworkTable.h>
#include <vision/VisionPipeline.h>
#include <vision/VisionRunner.h>
#include <wpi/StringRef.h>
#include <wpi/json.h>
#include <wpi/raw_istream.h>
#include <wpi/raw_ostream.h>

#include "cameraserver/CameraServer.h"

#include <opencv2/opencv.hpp>

#include <iostream>
#include <vector>
#include <array>
#include <thread>
#include <chrono>
#include <type_traits>

#include "extras/resources.h"
#include "extras/sighandle.h"
#include "extras/timing.h"
#include "extras/stats.h"

#include "visioncamera.h"

extern "C" void memcpy_threshold_asm(uint8_t* dest, const uint8_t* src, int size, int minimum);
extern "C" void memcpy_threshold_binary_asm(uint8_t* dest, const uint8_t* src, int size, int minimum);
extern "C" void memcpy_subtract_asm(uint8_t* base, uint8_t* sub, uint8_t* dest, int size);

CE_STR _default = "/boot/frc.json";

//typedef void(*vpipeline_f)(cs::CvSink, cs::CvSource);

//#define SWITCHED_CAMERA_CONFIG // enable this to setup switched cameras from the config file, otherwise the program starts a single switched camera for vision

//template<class video_t>
bool readConfig(std::vector<VisionCamera>& cameras, const char* file = _default) {
	//static_assert(std::is_base_of<cs::VideoSource, video_t>::value, "Template argument (video_t) must inherit from cs::VideoSource");

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

    try {
        for(const wpi::json::value_type& camera : j.at("cameras")) {
            //wpi::outs() << "Reading camera: " << camera << newline;
            // check for duplicates
			// cs::UsbCamera cam;
			// try {cam = cs::UsbCamera(camera.at("name").get<std::string>(), camera.at("path").get<std::string>());}
			// catch (const wpi::json::exception& e) {
			// 	wpi::errs() << "Config error in " << file << ": could not read camera name and/or path: " << e.what() << newline;
			// 	return false;
			// }
			// cam.SetConfigJson(camera);
			// cam.SetConnectionStrategy(cs::VideoSource::kConnectionKeepOpen);
            // wpi::outs() << "Added camera '" << cam.GetName() << "' on " << cam.GetPath() << newline;
			cameras.emplace_back(camera);
        }
    }
    catch (const wpi::json::exception& e) {
        wpi::errs() << "config error in " << file << ": could not read cameras: " << e.what() << newline;
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

cs::CvSink switchedCameraVision(
	const std::vector<VisionCamera>& cameras, 
	std::shared_ptr<nt::NetworkTable> table = nt::NetworkTableInstance::GetDefault().GetTable("Vision")
) {
	if(!table->ContainsKey("Vision Camera Index")) {
		table->PutNumber("Vision Camera Index", 0);
	}
	if(!table->ContainsKey("Vision Cameras Available")) {
		table->PutNumber("Vision Cameras Available", cameras.size());
	}
	static cs::CvSink source;
	if(cameras.size() > 0) {
		source = cameras[0].getVideo();
	}
	table->GetEntry("Vision Camera Index").AddListener(
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

template<typename num_t>
void addNetTableVar(num_t& var, const wpi::Twine& name, std::shared_ptr<nt::NetworkTable> table) {
	static_assert(std::is_arithmetic<num_t>::value, "num_t must be a numeric type");
	if(!table->ContainsKey(name)) {
		table->PutNumber(name.str(), var);
	} else {}
	table->GetEntry(name).AddListener(
		[&var](const nt::EntryNotification& event){
			if(event.value->IsDouble()) {
				var = event.value->GetDouble();
				//std::cout << "Networktable var(num) updated to : " << var << newline;
			}
		}, 
		NT_NOTIFY_IMMEDIATE | NT_NOTIFY_NEW | NT_NOTIFY_UPDATE
	);
}
void addNetTableVar(bool& var, const wpi::Twine& name, std::shared_ptr<nt::NetworkTable> table) {
	if(!table->ContainsKey(name)) {
		table->PutBoolean(name.str(), var);
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

StopWatch runtime("Runtime", &std::cout, 0);

void on_exit() {
	runtime.end();
}

int main(int argc, char* argv[]) {
	runtime.setStart();
	SigHandle::get();
	atexit(on_exit);

	std::vector<VisionCamera> cameras;

	if(argc > 1 && readConfig(cameras, argv[1])) {}
	else if(readConfig(cameras)) {}
	else { return EXIT_FAILURE; }

	std::shared_ptr<nt::NetworkTable> dash = nt::NetworkTableInstance::GetDefault().GetTable("Vision");

	std::array<bool, 4> options = {false, false, false, false};
	double weight = 0.5;
	uint8_t threshold = 100;
	size_t scale = 4;

	addNetTableVar(weight, "Weight (Red and Green)", dash);
	addNetTableVar(threshold, "Threshold (For weighted)", dash);
	addNetTableVar(options[0], "ASM Subtraction (Not working)", dash);
	addNetTableVar(options[1], "ASM Thresholding", dash);
	addNetTableVar(options[2], "Morphological Operations", dash);
	addNetTableVar(options[3], "Vision Multiview", dash);

	VisionCamera vision = cameras[0];

	//std::cout << "VISION CAMERA: " << vision.getWidth() << 'x' << vision.getHeight() << newline;

	cs::CvSink input = switchedCameraVision(cameras);
	cs::CvSource processed = vision.getSeparateServer();

	vision.setBrightnessAdjustable(dash);
	vision.setWhiteBalanceAdjustable(dash);
	vision.setExposureAdjustable(dash);

	// vision.setBrightness(50);
	// vision.setWhiteBalance(-1);
	// vision.setExposure(-1);

	cv::Mat frame(vision.getHeight(), vision.getWidth(), CV_8UC3);
	cv::Mat buffer(frame.size().height/scale, frame.size().width/scale, CV_8UC3);
	cv::Mat full(frame.size().height, (frame.size().width*(2.0+1.0/scale)), CV_8UC3);
	cv::Mat vertical = cv::Mat::zeros(frame.size().height, buffer.size().width, CV_8UC3);
	cv::Mat no_targets = cv::Mat::zeros(frame.size(), CV_8UC3);
	cv::Mat binary(buffer.size(), CV_8U);
	cv::Mat channels[3] = {
		cv::Mat(buffer.size(), CV_8U),
		cv::Mat(buffer.size(), CV_8U),
		cv::Mat(buffer.size(), CV_8U)
	};

	cv::putText(
		no_targets, "NO TARGETS DETECTED", 
		cv::Point(no_targets.size().width*0.2, no_targets.size().height*0.5), 
		cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 255), 2, cv::LINE_AA
	);

	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Point> poly_contour;
	cv::Rect boundingbox;

	double largest = 0.f;
	int16_t target = 0;

	uint64_t tframes = 0.f, fframes = 0.f;				// total frames and "final" frames
	double ttime = 0.f, ftime = 0.f, ltime = 0.f;		// total time and "final" time
	double fps_1s = 0.f, fps = 0.f;
	CHRONO::high_resolution_clock::time_point start = CHRONO::high_resolution_clock::now();
	for (;;) {
		input.GrabFrame(frame);

		cv::resize(frame, buffer, cv::Size(), 1.0/scale, 1.0/scale);

		cv::split(buffer, channels);
		cv::addWeighted(channels[1], weight, channels[2], weight, 0.0, binary);
		if(options[0]) { memcpy_subtract_asm(channels[0].data, binary.data, binary.data, binary.size().area()); }
		else { cv::subtract(channels[0], binary, binary); }

		if(options[1]) { memcpy_threshold_binary_asm(binary.data, binary.data, binary.size().area(), threshold); }
		else { cv::inRange(binary, cv::Scalar(threshold), cv::Scalar(255), binary); }

		if(options[3]) {
			cv::erode(binary, binary, cv::getStructuringElement(cv::MORPH_ERODE, cv::Size(3, 3)));
			cv::dilate(binary, binary, cv::getStructuringElement(cv::MORPH_DILATE, cv::Size(3, 3)));
		}

		contours.clear();
		poly_contour.clear();
		largest = 0.0;
		target = -1;

		cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);	// RETR_TREE (original)

		double area;
		for(size_t i = 0; i < contours.size(); i++) {
			area = cv::contourArea(contours[i]);
			if(area > largest) {
				largest = area;
				target = i;
			}
		}

		if(target >= 0) {
			boundingbox = cv::boundingRect(contours[target]);

			if(options[3]) {
				cv::rectangle(buffer, boundingbox.tl(), boundingbox.br(), cv::Scalar(255, 0, 0), 1);

				cv::cvtColor(binary, binary, cv::COLOR_GRAY2BGR, 3);
				cv::vconcat(binary, buffer, vertical);
				cv::vconcat(vertical, cv::Mat::zeros(frame.size().height - vertical.size().height, vertical.size().width, CV_8UC3), vertical);
				cv::hconcat(frame, vertical, full);
			}
			
			cv::Point2i tl = boundingbox.tl(), br = boundingbox.br();
			cv::rectangle(frame, cv::Point2i(tl.x*scale, tl.y*scale), cv::Point(br.x*scale, br.y*scale), cv::Scalar(255, 0, 0), 2);
		} else {
			if(options[3]) {
				cv::cvtColor(binary, binary, cv::COLOR_GRAY2BGR, 3);
				cv::vconcat(binary, cv::Mat::zeros(frame.size().height - binary.size().height, binary.size().width, CV_8UC3), vertical);
				cv::hconcat(frame, vertical, full);

				frame = no_targets;
			}

			cv::putText(
				frame, "NO TARGETS DETECTED", 
				cv::Point(frame.size().width*0.2, frame.size().height*0.5), 
				cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 255), 2, cv::LINE_AA
			);
		}

		cv::putText(
			frame, "CPU: " + std::to_string(CPU::get().refPercent()*100.f) + "% | " + std::to_string(CPU::temp()) + "*C", 
			cv::Point(0, 15), 
			cv::FONT_HERSHEY_DUPLEX, 0.45, cv::Scalar(0, 255, 0), 1, cv::LINE_AA
		);
		cv::putText(
			frame, "FPS (1F): " + std::to_string(fps), 
			cv::Point(0, 30), 
			cv::FONT_HERSHEY_DUPLEX, 0.45, cv::Scalar(0, 255, 0), 1, cv::LINE_AA
		);
		cv::putText(
			frame, "FPS (1S): " + std::to_string(fps_1s), 
			cv::Point(0, 45), 
			cv::FONT_HERSHEY_DUPLEX, 0.45, cv::Scalar(0, 255, 0), 1, cv::LINE_AA
		);
		cv::putText(
			frame, "Total Frames: " + std::to_string(tframes), 
			cv::Point(0, 60), 
			cv::FONT_HERSHEY_DUPLEX, 0.45, cv::Scalar(0, 255, 0), 1, cv::LINE_AA
		);

		if(options[3]) {
			cv::hconcat(full, frame, full);
			processed.PutFrame(full);
		} else {
			processed.PutFrame(frame);
		}

		tframes++;
		ttime = CHRONO::duration<double>(CHRONO::high_resolution_clock::now() - start).count();
		fps = 1.f/(ttime-ltime);
		if ((int)ttime > (int)ftime) {
			fps_1s = ((tframes - fframes) / (ttime - ftime));
			ftime = ttime;
			fframes = tframes;
		}
		ltime = ttime;
	}
}

// LIST OF THINGS
/*
- Dynamic resizing/scaling
- Position math -> networktables
- GET NETWORKTABLES TO WORK
- multiple cameras -> switching (find out what we want to do)
- compression/stay under bandwidth limit
- Modularize?
- MORE CUSTOM ASSEMBLY!!! :)
*/