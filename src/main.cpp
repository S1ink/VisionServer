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

	cs::UsbCamera* vcam;

	if (vconfig.cameras.size() > 0) {
		vcam = &vconfig.cameras[0].camera;
	}
	else {
		*vcam = frc::CameraServer::GetInstance()->StartAutomaticCapture();
	}

	std::cout << "Preset brightness: " << vcam->GetBrightness() << newline;

	cs::VideoMode v_info = vcam->GetVideoMode();

	nt::NetworkTableInstance netinst = nt::NetworkTableInstance::GetDefault();
	std::shared_ptr<nt::NetworkTable> dash = netinst.GetTable("SmartDashboard");
	/*if (!dash->GetNumber("Brightness", 0)) {
		dash->PutNumber("Brightness", 50);
	}*/
	dash->GetEntry("Brightness").AddListener(
		[&vcam](const nt::EntryNotification& event) {
			if (event.value->IsDouble()) {
				int i = event.value->GetDouble();
				vcam->SetBrightness(i < 0 ? 0 : (i > 100 ? 100 : i));
			}
		}, 
		NT_NOTIFY_IMMEDIATE | NT_NOTIFY_NEW | NT_NOTIFY_UPDATE
	);
	/*if (!dash->GetNumber("WhiteBalance", 0)) {
		dash->PutNumber("WhiteBalance", 50);
	}*/
	dash->GetEntry("WhiteBalance").AddListener(
		[&vcam](const nt::EntryNotification& event) {
			if (event.value->IsDouble()) {
				int i = event.value->GetDouble();
				vcam->SetBrightness(i < 0 ? 0 : (i > 100 ? 100 : i));
			}
		},
		NT_NOTIFY_IMMEDIATE | NT_NOTIFY_NEW | NT_NOTIFY_UPDATE
	);
	/*if (!dash->GetNumber("Exposure", 0)) {
		dash->PutNumber("Exposure", 20);
	}*/
	dash->GetEntry("Exposure").AddListener(
		[&vcam](const nt::EntryNotification& event) {
			if (event.value->IsDouble()) {
				int i = event.value->GetDouble();
				vcam->SetExposureManual(i < 0 ? 0 : (i > 100 ? 100 : i));
			}
			else if (event.value->IsBoolean()) {
				event.value->GetBoolean() ? vcam->SetExposureAuto() : vcam->SetExposureHoldCurrent();
			}
		},
		NT_NOTIFY_IMMEDIATE | NT_NOTIFY_NEW | NT_NOTIFY_UPDATE
	);

	std::array<uint8_t, 6> thresholds = { 30, 90, 20, 230, 20, 230 };

	//dash->PutNumberArray("Thresholds", wpi::ArrayRef<double>());
	dash->GetEntry("Thresholds").AddListener(
		[&](const nt::EntryNotification& event) {
			if (event.value->IsDoubleArray()) {
				wpi::ArrayRef<double> array = event.value->GetDoubleArray();
				for (size_t i = 0; i < 6 && array.size(); i++) {
					thresholds[i] = event.value->GetDoubleArray()[i];
				}
			}
		},
		NT_NOTIFY_IMMEDIATE | NT_NOTIFY_NEW | NT_NOTIFY_UPDATE
	);

	cs::CvSink input = frc::CameraServer::GetInstance()->GetVideo(vcam->GetName());
	cs::CvSource processed = frc::CameraServer::GetInstance()->PutVideo("Processed Output", v_info.width, v_info.height);
	cs::CvSource thresholded = frc::CameraServer::GetInstance()->PutVideo("Thresholded Output", v_info.width, v_info.height);

	cv::Mat frame(v_info.height, v_info.width, CV_8UC3);
	cv::Mat buffer = frame;
	cv::Mat binary(frame.size(), CV_8U);
	cv::Mat lines = cv::Mat::zeros(frame.size(), CV_8UC3);

	cv::Moments moments;
	double dM01, dM10, dArea;
	int posX, posY, iLastX = 0, iLastY = 0;
	std::vector<cv::Mat> vec_channels;

	uint64_t tframes = 0, fframes = 0;	// total frames and "final" frames
	double ttime = 0, ftime = 0;		// total time and "final" time
	CHRONO::high_resolution_clock::time_point start = CHRONO::high_resolution_clock::now();
	for (;;) {
		input.GrabFrame(frame);

		cv::cvtColor(frame, frame, cv::COLOR_BGR2HSV);									// convert to HSV colorspace
		cv::inRange(frame, cv::Scalar(thresholds[0], thresholds[2], thresholds[4]), cv::Scalar(thresholds[1], thresholds[3], thresholds[5]), binary);	// threshold based on HSV colorspace

		cv::erode(binary, binary, cv::getStructuringElement(cv::MORPH_ERODE, cv::Size(3, 3)));
		cv::dilate(binary, binary, cv::getStructuringElement(cv::MORPH_DILATE, cv::Size(3, 3)));

		thresholded.PutFrame(binary);													// display the thresholded image

		//cv::cvtColor(buffer, frame, cv::COLOR_);
		moments = cv::moments(binary, true);

		dM01 = moments.m01;
		dM10 = moments.m10;
		dArea = moments.m00;

		if (dArea > 10000) {
			posX = dM10 / dArea;
			posY = dM01 / dArea;

			if (iLastX >= 0 && iLastY >= 0 && posX >= 0 && posY >= 0) {
				cv::line(lines, cv::Point(posX, posY), cv::Point(iLastX, iLastY), cv::Scalar(0, 255, 0), 2);
			}

			iLastX = posX;
			iLastY = posY;
		}

		cv::cvtColor(frame, frame, cv::COLOR_HSV2BGR);

		lines.resize(frame.rows);
		frame = frame + lines;

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
