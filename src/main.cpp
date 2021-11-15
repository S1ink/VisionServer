#include <networktables/NetworkTableInstance.h>
#include <networktables/EntryListenerFlags.h>
#include <networktables/NetworkTableEntry.h>
#include <networktables/NetworkTable.h>
#include "cameraserver/CameraServer.h"
#include <vision/VisionPipeline.h>
#include <vision/VisionRunner.h>

#include <opencv2/opencv.hpp>

#include "extras/resources.h"
#include "extras/sighandle.h"
#include "extras/timing.h"

#include <iostream>
#include <vector>
#include <array>
#include <thread>
#include <chrono>
#include <type_traits>

#include "cameraconfig.h"

//class TestPipeline : public frc::VisionPipeline {
//private:
//    cv::Mat buffer;
//    cs::CvSource source;
//
//public:
//
//	TestPipeline(const CameraConfig& config) : 
//		source(frc::CameraServer::GetInstance()->PutVideo(config.name, config.camera.GetVideoMode().width, config.camera.GetVideoMode().height)) {}
//
//	void Process(cv::Mat& mat) override {
//
//		this->buffer = mat;
//		cv::cvtColor(mat, buffer, cv::COLOR_BGR2HSV_FULL);
//		cv::threshold(buffer, mat, 0, 255, cv::THRESH_BINARY);
//
//		this->source.PutFrame(mat);
//
//		//std::cout << "processing done?";
//
//	}
//
//	void output() {
//		
//	}
//
//	static void startThread(const CameraConfig& config) {
//		std::thread(
//			[&] {
//				frc::VisionRunner<TestPipeline> runner(
//					config.camera,
//					new TestPipeline(config),
//					[&](TestPipeline& pipeline) {
//						//pipeline.output();
//					}
//				);
//				runner.RunForever();
//			}
//		).detach();
//	}
//
//};

template<typename num_t>
void addNetTableVar(num_t& var, const wpi::Twine& name, std::shared_ptr<nt::NetworkTable> table) {
	static_assert(std::is_arithmetic<num_t>::value, "num_t must be a number type");
	if(!table->ContainsKey(name)) {
		table->PutNumber(name.str(), var);
	} else {}
	table->GetEntry(name).AddListener(
		[&var](const nt::EntryNotification& event){
			if(event.value->IsDouble()) {
				var = event.value->GetDouble();
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

	StreamConfig vconfig;
	if(argc > 1 && vconfig.parse(argv[1])) {}	// parse the config file and set variables 
	else if(vconfig.parse()) {}
	else { return EXIT_FAILURE; }
	vconfig.setup();							// start networktables and begin camera streaming

	cs::UsbCamera* vcam = nullptr;

	if (vconfig.cameras.size() > 0) {
		vcam = &vconfig.cameras[0].camera;
	}
	else {
		static cs::UsbCamera global = frc::CameraServer::GetInstance()->StartAutomaticCapture();
		vcam = &global;
	}

	cs::VideoMode v_info = vcam->GetVideoMode();

	nt::NetworkTableInstance netinst = nt::NetworkTableInstance::GetDefault();
	std::shared_ptr<nt::NetworkTable> dash = netinst.GetTable("SmartDashboard");

	bool morph = false;
	std::array<uint8_t, 6> thresh = {80, 150, 150, 120, 255, 255};

	addNetTableVar(morph, "Morph-ops", dash);
	addNetTableVar(thresh[0], "low-hue", dash);
	addNetTableVar(thresh[1], "low-sat", dash);
	addNetTableVar(thresh[2], "low-val", dash);
	addNetTableVar(thresh[3], "high-hue", dash);
	addNetTableVar(thresh[4], "high-sat", dash);
	addNetTableVar(thresh[5], "high-val", dash);

	cs::CvSink input = frc::CameraServer::GetInstance()->GetVideo(vcam->GetName());
	cs::CvSource processed = frc::CameraServer::GetInstance()->PutVideo("Processed Output", v_info.width, v_info.height);
	cs::CvSource thresholded = frc::CameraServer::GetInstance()->PutVideo("Thresholded Output", v_info.width, v_info.height);

	cv::Mat frame(v_info.height, v_info.width, CV_8UC3);
	cv::Mat buffer = frame;
	cv::Mat binary(frame.size(), CV_8U);
	cv::Mat markup = cv::Mat::zeros(frame.size(), CV_8UC3);

	// cv::Moments moments;
	// double dM01, dM10, dArea;
	// int posX, posY, iLastX = 0, iLastY = 0;
	// std::vector<cv::Mat> vec_channels;

	uint64_t tframes = 0, fframes = 0;	// total frames and "final" frames
	double ttime = 0, ftime = 0;		// total time and "final" time
	CHRONO::high_resolution_clock::time_point start = CHRONO::high_resolution_clock::now();
	for (;;) {
		input.GrabFrame(frame);

		cv::cvtColor(frame, frame, cv::COLOR_BGR2HSV);									// convert to HSV colorspace
		cv::inRange(frame, cv::Scalar(thresh[0], thresh[1], thresh[2]), cv::Scalar(thresh[3], thresh[4], thresh[5]), binary);	// threshold based on HSV colorspace

		if(morph) {
			cv::erode(binary, binary, cv::getStructuringElement(cv::MORPH_ERODE, cv::Size(3, 3)));
			cv::dilate(binary, binary, cv::getStructuringElement(cv::MORPH_DILATE, cv::Size(3, 3)));
		}

		thresholded.PutFrame(binary);													// display the thresholded image

		std::vector<std::vector<cv::Point> > contours;
		cv::findContours(binary, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

		std::vector<std::vector<cv::Point> > contours_poly(contours.size());
		std::vector<cv::Rect> bound_rect(contours.size());

		double max_area = 0, comp = 0;
		int max_id = -1;

		for(size_t i = 0; i < contours.size(); i++) {
			cv::approxPolyDP(contours[i], contours_poly[i], 3, true);
			bound_rect[i] = cv::boundingRect(contours_poly[i]);

			comp = cv::contourArea(contours[i]);
			if(comp > max_area) {
				max_area = comp;
				max_id = i;
			}

			//cv::drawContours(markup, contours_poly, (int)i, cv::Scalar(0, 0, 255));
		}

		if(max_id >= 0) {
			cv::rectangle(markup, bound_rect[max_id].tl(), bound_rect[max_id].br(), cv::Scalar(0, 0, 255), 2);
		}

		//cv::cvtColor(buffer, frame, cv::COLOR_);
		// moments = cv::moments(binary, true);

		// dM01 = moments.m01;
		// dM10 = moments.m10;
		// dArea = moments.m00;

		// if (dArea > 10000) {
		// 	posX = dM10 / dArea;
		// 	posY = dM01 / dArea;

		// 	if (iLastX >= 0 && iLastY >= 0 && posX >= 0 && posY >= 0) {
		// 		cv::line(lines, cv::Point(posX, posY), cv::Point(iLastX, iLastY), cv::Scalar(0, 255, 0), 2);
		// 	}

		// 	iLastX = posX;
		// 	iLastY = posY;
		// }

		cv::cvtColor(frame, frame, cv::COLOR_HSV2BGR);

		markup.resize(frame.rows);
		frame = frame + markup;

		//markup.release();
		markup = cv::Mat::zeros(frame.size(), CV_8UC3);

		processed.PutFrame(frame);

		tframes++;
		ttime = CHRONO::duration<double>(CHRONO::high_resolution_clock::now() - start).count();
		if ((int)ttime > (int)ftime) {
			std::cout 
				<< tframes << " frames processed in " 
				<< ttime << " seconds -> Avg fps of " 
				<< ((tframes - fframes) / (ttime - ftime)) 
				<< newline;
			ftime = ttime;
			fframes = tframes;
		}
	}
}