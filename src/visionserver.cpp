#include "visionserver.h"

#include <networktables/EntryListenerFlags.h>
#include <networktables/NetworkTableEntry.h>

#include "extras/stats.h"
//#include "pipelines.h"
#include "vision.h"

// PipelineBase::PipelineBase(VisionServer* server) : env(server) {}

// void PipelineBase::process(cv::Mat& io_frame, bool thresh) {}

// CHRONO::high_resolution_clock::time_point PipelineBase::getEnvStart() {
// 	return this->env->start;
// }
// const cv::Mat_<float>& PipelineBase::getCameraMatrix() {
// 	return this->env->getCameraMatrix();
// }
// const cv::Mat_<float>& PipelineBase::getDistortion() {
// 	return this->env->getDistortion();
// }
// void PipelineBase::updateMatrices(const cv::Mat_<float>& tvec, const cv::Mat_<float>& rvec) {
// 	this->env->updateMatrices(tvec, rvec);
// }
// void PipelineBase::updateMatrices(const cv::Mat_<float>& tvec) {
// 	this->env->updateMatrices(tvec);
// }

VPipeline::VPipeline(VisionServer& server) : 
	table(nt::NetworkTableInstance::GetDefault().GetTable("PIPELINES")->GetSubTable("Unnamed Pipeline")), env(&server) 
{}
VPipeline::VPipeline(VisionServer& server, const char* ntable) : 
	table(nt::NetworkTableInstance::GetDefault().GetTable("PIPELINES")->GetSubTable(ntable)), env(&server) 
{}
VPipeline::VPipeline(VisionServer& server, const wpi::Twine& ntable) : 
	table(nt::NetworkTableInstance::GetDefault().GetTable("PIPELINES")->GetSubTable(ntable)), env(&server) 
{}

const cv::Mat_<float>& VPipeline::getCameraMatrix() const {
	return this->env->getCameraMatrix();
}
const cv::Mat_<float>& VPipeline::getCameraDistortion() const {
	return this->env->getDistortion();
}
void VPipeline::updateMatrices(const cv::Mat_<float>& tvec) {
	this->env->updateMatrices(tvec);
}
void VPipeline::updateMatrices(const cv::Mat_<float>& tvec, const cv::Mat_<float>& rvec) {
	this->env->updateMatrices(tvec, rvec);
}
const std::shared_ptr<nt::NetworkTable> VPipeline::getTable() {
	return this->table;
}
const VisionServer* VPipeline::getEnv() const {
	return this->env;
}

DefaultPipeline::DefaultPipeline(VisionServer& server) : VPipeline(server, "Default Pipeline") {}



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
	//if(!this->table->ContainsKey("Stream Quality")) {
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
					cameras[idx].getCameraMatrix(this->camera_matrix);
					cameras[idx].getDistortion(this->distortion);
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
void VisionServer::setCompression(int8_t quality) {
	this->stream.SetCompression(quality > 100 ? 100 : (quality < -1 ? -1 : quality));
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

const cv::Mat_<float>& VisionServer::getCameraMatrix() const {
	return this->camera_matrix;
}
const cv::Mat_<float>& VisionServer::getDistortion() const {
	return this->distortion;
}

void VisionServer::updateMatrices(const cv::Mat_<float>& tvec, const cv::Mat_<float>& rvec) {}
void VisionServer::updateMatrices(const cv::Mat_<float>& tvec) {}



// void VisionServer::runVisionTest(int8_t quality) {	
// 	this->stream.SetCompression(quality);

// 	SquareTargetPNP p1(this);
// 	BBoxDemo p2(this);
// 	PipelineBase p3(this);
// 	PipelineBase pbase(this);
	
// 	this->table->PutNumber("Pipeline Index", 0);
// 	this->table->PutNumber("Pipelines Available", 3);
// 	this->table->PutBoolean("Show Threshold", false);
// 	this->table->PutBoolean("Show Statistics", false);

// 	cv::Mat frame(this->getCurrentResolution(), CV_8UC3);

// 	this->start = CHRONO::high_resolution_clock::now();
// 	while(this->runloop) {
// 		std::cout << "Processing?\n";
// 		this->source.GrabFrame(frame);
// 		switch((int8_t)this->table->GetEntry("Pipeline Index").GetDouble(-1)) {
// 			case 0 : {
// 				this->beg = CHRONO::high_resolution_clock::now();
// 				p1.process(frame, this->table->GetEntry("Show Threshold").GetBoolean(false));
// 				this->end = CHRONO::high_resolution_clock::now();
// 				break;
// 			}
// 			case 1 : {
// 				this->beg = CHRONO::high_resolution_clock::now();
// 				p2.process(frame, this->table->GetEntry("Show Threshold").GetBoolean(false));
// 				this->end = CHRONO::high_resolution_clock::now();
// 				break;
// 			}
// 			case 2 : {
// 				this->beg = CHRONO::high_resolution_clock::now();
// 				p3.process(frame, this->table->GetEntry("Show Threshold").GetBoolean(false));
// 				this->end = CHRONO::high_resolution_clock::now();
// 			}
// 			case -1 :
// 			default : {
// 				std::cout << "Pipline index out of bounds, please only use values >0 and <'Piplines Available'\n";
// 				this->beg = CHRONO::high_resolution_clock::now();
// 				PipelineBase(this).process(frame, this->table->GetEntry("Show Threshold").GetBoolean(false));
// 				this->end = CHRONO::high_resolution_clock::now();
// 			}
// 		}
// 		this->total_frames++;

// 		this->total_time = CHRONO::duration<double>(this->end - this->start).count();
// 		this->frame_time = CHRONO::duration<double>(this->end - this->beg).count();
// 		this->loop_time = CHRONO::duration<double>(this->end - this->last).count();
// 		this->last = this->end;

// 		this->fps = 1.f/this->loop_time;
// 		if((int)this->total_time > (int)this->sec1_time) {
// 			this->fps_1s = ((this->total_frames - this->sec1_frames) / (this->total_time - this->sec1_time));
// 			this->sec1_time = this->total_time;
// 			this->sec1_frames = this->total_frames;
// 		}

// 		if(this->table->GetEntry("Show Statistics").GetBoolean(false)) {
// 			this->putStats(frame);
// 		}
// 		this->output.PutFrame(frame);
// 	}
// 	this->runloop = true;
	
// 	this->table->Delete("Pipeline Index");
// 	this->table->Delete("Piplines Available");
// 	this->table->Delete("Show Threshold");
// 	this->table->Delete("Show Statistics");
// }