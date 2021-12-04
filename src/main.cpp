#include <networktables/NetworkTableInstance.h>
#include <networktables/EntryListenerFlags.h>
#include <networktables/NetworkTableEntry.h>
#include <networktables/NetworkTable.h>
// #include <vision/VisionPipeline.h>
// #include <vision/VisionRunner.h>
#include <wpi/StringRef.h>
#include <wpi/json.h>
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

//#define SWITCHED_CAMERA_CONFIG // enable this to setup switched cameras from the config file, otherwise the program starts a single switched camera for vision

#include "visioncamera.h"
#include "visionserver.h"

extern "C" void memcpy_threshold_asm(uint8_t* dest, const uint8_t* src, int size, int minimum);
extern "C" void memcpy_threshold_binary_asm(uint8_t* dest, const uint8_t* src, int size, int minimum);
extern "C" void memcpy_subtract_asm(uint8_t* base, uint8_t* sub, uint8_t* dest, int size);

//typedef void(*vpipeline_f)(cs::CvSink, cs::CvSource);

// cs::CvSink switchedCameraVision(
// 	const std::vector<VisionCamera>& cameras, 
// 	std::shared_ptr<nt::NetworkTable> table = nt::NetworkTableInstance::GetDefault().GetTable("Vision")
// ) {
// 	if(!table->ContainsKey("Vision Camera Index")) {
// 		table->PutNumber("Vision Camera Index", 0);
// 	}
// 	if(!table->ContainsKey("Vision Cameras Available")) {
// 		table->PutNumber("Vision Cameras Available", cameras.size());
// 	}
// 	static cs::CvSink source;
// 	if(cameras.size() > 0) {
// 		source = cameras[0].getVideo();
// 	}
// 	table->GetEntry("Vision Camera Index").AddListener(
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

template<typename num_t>
cv::Size_<num_t> operator/(cv::Size_<num_t> input, size_t scale) {
	static_assert(std::is_arithmetic<num_t>::value, "Template parameter (num_t) must be arithemetic type");
	return cv::Size_<num_t>(input.width/scale, input.height/scale);
}

// DONT COPY INSTANCES THEY ARE EXTREMELY HEAVY
class TestPipeline : public PipelineBase {
public:
	TestPipeline(VisionServer* server) : PipelineBase(server) {
		addNetTableVar(this->weight, "Weight (Red and Green)", this->table);
		addNetTableVar(this->threshold, "Threshold (For weighted)", this->table);
		addNetTableVar(this->options[0], "Morphological Operations", this->table);

		this->resizeBuffers(this->env->getCurrentResolution());
	}
	TestPipeline(const TestPipeline& other) = delete;	// only remove this if absolutely necessary

	inline bool cmpSize(const cv::Size& newsize) {
		return this->resolution == newsize;
	}
	void resizeBuffers(cv::Size size) {
		this->buffer = cv::Mat(size/this->scale, CV_8UC3);
		this->binary = cv::Mat(size/this->scale, CV_8UC1);
		for(size_t i = 0; i < this->channels.size(); i++) {
			channels[i] = cv::Mat(size/this->scale, CV_8UC1);
		}
	}

	void process(cv::Mat& frame, cs::CvSource& output) override {
		this->beg = CHRONO::high_resolution_clock::now();

		if(cmpSize(frame.size())) {
			this->resizeBuffers(frame.size());
		}

		cv::resize(frame, this->buffer, cv::Size(), 1.0/this->scale, 1.0/this->scale);
		cv::split(this->buffer, this->channels);
		cv::addWeighted(this->channels[1], this->weight, this->channels[2], this->weight, 0.0, this->binary);
		cv::subtract(this->channels[0], this->binary, this->binary);
		memcpy_threshold_binary_asm(this->binary.data, this->binary.data, this->binary.size().area(), this->threshold);

		if(options[0]) {
			cv::erode(this->binary, this->binary, cv::getStructuringElement(cv::MORPH_ERODE, cv::Size(3, 3)));
			cv::dilate(this->binary, this->binary, cv::getStructuringElement(cv::MORPH_DILATE, cv::Size(3, 3)));
		}

		this->contours.clear();
		this->poly_contour.clear();
		this->largest = 0.f;
		this->target = -1;

		cv::findContours(this->binary, this->contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

		for(size_t i = 0; i < this->contours.size(); i++) {
			this->area = cv::contourArea(this->contours[i]);
			if(this->area > this->largest) {
				this->largest = this->area;
				this->target = i;
			}
		}

		if(this->target >= 0) {
			this->boundingbox = cv::boundingRect(this->contours[this->target]);
			cv::Point2i tl = this->boundingbox.tl(), br = this->boundingbox.br();
			cv::rectangle(frame, cv::Point2i(tl.x*this->scale, tl.y*this->scale), cv::Point(br.x*this->scale, br.y*this->scale), cv::Scalar(255, 0, 0), 2);
		} else {
			cv::putText(
				frame, "NO TARGETS DETECTED", 
				cv::Point(frame.size().width*0.2, frame.size().height*0.5), 
				cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 255), 2, cv::LINE_AA
			);

			// DEBUG
			// cv::putText(
			// 	frame, "CONTOURS: " + std::to_string(this->contours.size()), 
			// 	cv::Point(frame.size().width*0.2, frame.size().height*0.5), 
			// 	cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 255), 2, cv::LINE_AA
			// );
		}

		cv::putText(
			frame, "CPU: " + std::to_string(CPU::get().refPercent()*100.f) + "% | " + std::to_string(CPU::temp()) + "*C", 
			cv::Point(0, 15), 
			cv::FONT_HERSHEY_DUPLEX, 0.45, cv::Scalar(0, 255, 0), 1, cv::LINE_AA
		);
		cv::putText(
			frame, "Frametime: " + std::to_string(this->frame_time) + " ms", 
			cv::Point(0, 30), 
			cv::FONT_HERSHEY_DUPLEX, 0.45, cv::Scalar(0, 255, 0), 1, cv::LINE_AA
		);
		cv::putText(
			frame, "Looptime: " + std::to_string(this->loop_time) + " ms", 
			cv::Point(0, 45), 
			cv::FONT_HERSHEY_DUPLEX, 0.45, cv::Scalar(0, 255, 0), 1, cv::LINE_AA
		);
		cv::putText(
			frame, "FPS (1F): " + std::to_string(this->fps), 
			cv::Point(0, 60), 
			cv::FONT_HERSHEY_DUPLEX, 0.45, cv::Scalar(0, 255, 0), 1, cv::LINE_AA
		);
		cv::putText(
			frame, "FPS (1S): " + std::to_string(this->fps_1s), 
			cv::Point(0, 75), 
			cv::FONT_HERSHEY_DUPLEX, 0.45, cv::Scalar(0, 255, 0), 1, cv::LINE_AA
		);
		cv::putText(
			frame, "Total Frames: " + std::to_string(this->total_frames), 
			cv::Point(0, 90), 
			cv::FONT_HERSHEY_DUPLEX, 0.45, cv::Scalar(0, 255, 0), 1, cv::LINE_AA
		);

		output.PutFrame(frame);
		this->end = CHRONO::high_resolution_clock::now();
		this->total_frames++;

		this->total_time = CHRONO::duration<double>(this->end - this->getEnvStart()).count();
		this->frame_time = CHRONO::duration<double>(this->end - this->beg).count();
		this->loop_time = CHRONO::duration<double>(this->end - this->last).count();
		this->last = this->end;

		this->fps = 1.f/this->loop_time;
		if ((int)this->total_time > (int)this->sec1_time) {
			this->fps_1s = ((this->total_frames - this->sec1_frames) / (this->total_time - this->sec1_time));
			sec1_time = total_time;
			sec1_frames = total_frames;
		}
	}

private: 
	std::shared_ptr<nt::NetworkTable> table{nt::NetworkTableInstance::GetDefault().GetTable("Test Pipeline")};
	std::array<bool, 1> options{false};
	double weight{0.5};
	uint8_t threshold{100};
	size_t scale{4};

	cv::Size resolution;
	cv::Mat buffer, binary;
	std::array<cv::Mat, 3> channels;

	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Point> poly_contour;
	cv::Rect boundingbox;
	double largest{0.f}, area{0.f};
	int16_t target{0};

	uint64_t total_frames{0}, sec1_frames{0};
	double total_time{0.f}, frame_time{0.f}, loop_time{0.f}, sec1_time{0.f}, fps_1s{0.f}, fps{0.f};
	CHRONO::high_resolution_clock::time_point beg, end, last;

};

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

	VisionServer server(cameras);
	server.runVision<TestPipeline>();
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