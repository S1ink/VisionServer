#include "visionserver.h"
#include "vision.h"

PipelineBase::PipelineBase(const VisionServer* server) : env(server) {}

void PipelineBase::process(cv::Mat& io_frame, bool thresh) {}
CHRONO::high_resolution_clock::time_point PipelineBase::getEnvStart() {
	return this->env->start;
}

VisionServer::VisionServer(std::vector<VisionCamera>& cameras) : cameras(&cameras) {
	for(size_t i = 0; i < cameras.size(); i++) {
		cameras[i].setNetworkAdjustable();
	}

    //if(!this->table->ContainsKey("Camera Index")) {
		this->table->PutNumber("Camera Index", 0);
	//}
	//if(!this->table->ContainsKey("Cameras Available")) {
		this->table->PutNumber("Cameras Available", cameras.size());
	//}
	//if(!this->table->ContainsKey("Stream Compression")) {
		//this->table->PutNumber("Stream Quality", 100);
	//}

	this->source = cameras[0].getVideo();
	this->output = cs::CvSource("Processed Output", cameras[0].GetVideoMode());
	this->stream = frc::CameraServer::GetInstance()->AddServer("Vision Output");
	this->stream.SetSource(this->output);
	
	table->GetEntry("Camera Index").AddListener(
		[&cameras, this](const nt::EntryNotification& event) {
			if(event.value->IsDouble()) {
				size_t idx = event.value->GetDouble();
				if(idx >= 0 && idx < cameras.size()) {
					this->source.SetSource(cameras[idx]);
					//this->stream.SetConfigJson(cameras[idx].getStreamJson());
				}
			}
		},
		NT_NOTIFY_IMMEDIATE | NT_NOTIFY_NEW | NT_NOTIFY_UPDATE
	);
	// table->GetEntry("Stream Quality").AddListener(
	// 	[this](const nt::EntryNotification& event) {
	// 		if(event.value->IsDouble()) {
	// 			int8_t val = event.value->GetDouble();
	// 			this->stream.SetCompression(val > 100 ? 100 : (val < -1 ? -1 : val));
	// 			//std::cout << "SET Quality?: " << (int16_t)val << newline;
	// 		}
	// 	},
	// 	NT_NOTIFY_IMMEDIATE | NT_NOTIFY_NEW | NT_NOTIFY_UPDATE
	// );
}

size_t VisionServer::validIndexes() const {
    if(this->cameras) {
        return this->cameras->size();
    }
    return 0;
}
bool VisionServer::setCamera(size_t idx) {
    if(idx < this->validIndexes()) {
        //this->source.SetSource(this->cameras->operator[](idx));   // this should be taken care of by the networktables callback
        this->table->GetEntry("Camera Index").SetDouble(idx);
    }
    return false;
}
cv::Size VisionServer::getCurrentResolution() const {
	return getResolution(this->source.GetSource().GetVideoMode());
}

bool VisionServer::stopVision() {
	this->runloop = false;
	if(this->launched.joinable()) {
		this->launched.join();
		return true;
	}
	return false;
}

void VisionServer::putStats(cv::Mat& io_frame) {
	cv::putText(
		io_frame, "CPU: " + std::to_string(CPU::get().refPercent()*100.f) + "% | " + std::to_string(CPU::temp()) + "*C", 
		cv::Point(0, 15), 
		cv::FONT_HERSHEY_DUPLEX, 0.45, cv::Scalar(0, 255, 0), 1, cv::LINE_AA
	);
	cv::putText(
		io_frame, "Frametime: " + std::to_string(this->frame_time*1000) + " ms", 
		cv::Point(0, 30), 
		cv::FONT_HERSHEY_DUPLEX, 0.45, cv::Scalar(0, 255, 0), 1, cv::LINE_AA
	);
	cv::putText(
		io_frame, "Looptime: " + std::to_string(this->loop_time*1000) + " ms", 
		cv::Point(0, 45), 
		cv::FONT_HERSHEY_DUPLEX, 0.45, cv::Scalar(0, 255, 0), 1, cv::LINE_AA
	);
	cv::putText(
		io_frame, "FPS (1F): " + std::to_string(this->fps), 
		cv::Point(0, 60), 
		cv::FONT_HERSHEY_DUPLEX, 0.45, cv::Scalar(0, 255, 0), 1, cv::LINE_AA
	);
	cv::putText(
		io_frame, "FPS (1S): " + std::to_string(this->fps_1s), 
		cv::Point(0, 75), 
		cv::FONT_HERSHEY_DUPLEX, 0.45, cv::Scalar(0, 255, 0), 1, cv::LINE_AA
	);
	cv::putText(
		io_frame, "Total Frames: " + std::to_string(this->total_frames), 
		cv::Point(0, 90), 
		cv::FONT_HERSHEY_DUPLEX, 0.45, cv::Scalar(0, 255, 0), 1, cv::LINE_AA
	);
}