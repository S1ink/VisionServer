#include "visionserver.h"

#include <networktables/EntryListenerFlags.h>
#include <networktables/NetworkTableEntry.h>

#include "extras/stats.h"
//#include "pipelines.h"
#include "vision.h"

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
void VPipeline::updateTarget(const std::string& target) {
	this->env->updateTarget(target);
}
// void VPipeline::updateMatrices(const cv::Mat_<float>& tvec) {
// 	this->env->updateMatrices(tvec);
// }
// void VPipeline::updateMatrices(const cv::Mat_<float>& tvec, const cv::Mat_<float>& rvec) {
// 	this->env->updateMatrices(tvec, rvec);
// }
const std::shared_ptr<nt::NetworkTable> VPipeline::getTable() const {
	return this->table;
}
const VisionServer* VPipeline::getEnv() const {
	return this->env;
}

DefaultPipeline::DefaultPipeline(VisionServer& server) : VPipeline(server, "Default Pipeline") {}



// Position& Position::Get() {
// 	static Position global;
// 	return global;
// }
// void Position::setPos(float x, float y, float z) {
// 	this->pos_table->PutNumber("x", x);
// 	this->pos_table->PutNumber("y", y);
// 	this->pos_table->PutNumber("z", z);
// }
// void Position::setDistance(double d) {
// 	this->pos_table->PutNumber("distance", d);
// }
// void Position::setThetaUD(double deg) {
// 	this->pos_table->PutNumber("up-down", deg);
// }
// void Position::setThetaLR(double deg) {
// 	this->pos_table->PutNumber("right-left", deg);
// }
// void Position::setAll(const cv::Mat_<float>& tvec) {
// 	this->pos_table->PutNumber("x", tvec[0][0]);
// 	this->pos_table->PutNumber("y", tvec[1][0]);
// 	this->pos_table->PutNumber("z", tvec[2][0]);
// 	this->pos_table->PutNumber("distance", sqrt(pow(tvec[0][0], 2) + pow(tvec[1][0], 2) + pow(tvec[2][0], 2)));
// 	this->pos_table->PutNumber("up-down", atan2(tvec[0][0], tvec[2][0])*180/M_PI);
// 	this->pos_table->PutNumber("right-left", atan2(tvec[1][0], tvec[2][0])*-180/M_PI);
// }



VisionServer::VisionServer(std::vector<VisionCamera>& cameras) : cameras(&cameras) {
	for(size_t i = 0; i < cameras.size(); i++) {
		cameras[i].setNetworkAdjustable();
	}
	this->table->PutNumber("Camera Index", 0);
	this->table->PutNumber("Cameras Available", cameras.size());

	this->source = cameras[0].getVideo();
	this->output = cs::CvSource("Vision Stream", cameras[0].GetVideoMode());
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
void VisionServer::updateStats() {
	std::shared_ptr<nt::NetworkTable> stats = this->table->GetSubTable("stats");
	stats->PutNumber("CPU Utilization(%-1F)", CPU::get().refPercent()*100.f);
	stats->PutNumber("CPU Temp(*C)", CPU::temp());
	stats->PutNumber("Frametime(ms)", this->frame_time*1000);
	stats->PutNumber("Looptime(ms)", this->loop_time*1000);
	stats->PutNumber("FPS(1F)", this->fps);
	stats->PutNumber("FPS(1s)", this->fps_1s);
	stats->PutNumber("Frames", this->total_frames);
}

void VisionServer::setupNtVflag() {
	this->table->PutBoolean("debug", false);
	this->table->PutBoolean("threshold", false);
	this->table->PutBoolean("demo", false);
}
uint8_t VisionServer::getNtVflag() {
	return (this->table->GetBoolean("debug", false) | this->table->GetBoolean("threshold", false) << 1 | this->table->GetBoolean("demo", false) << 2);
}
void VisionServer::deleteNtVflag() {
	this->table->Delete("debug");
	this->table->Delete("threshold");
	this->table->Delete("demo");
}

const cv::Mat_<float>& VisionServer::getCameraMatrix() const {
	return this->camera_matrix;
}
const cv::Mat_<float>& VisionServer::getDistortion() const {
	return this->distortion;
}

void VisionServer::updateTarget(const std::string& target) {
	this->active_target.setTarget(target);
}

// void VisionServer::updateMatrices(const cv::Mat_<float>& tvec, const cv::Mat_<float>& rvec) {

// }
// void VisionServer::updateMatrices(const cv::Mat_<float>& tvec) {
// 	std::shared_ptr<nt::NetworkTable> pos = nt::NetworkTableInstance::GetDefault().GetTable("robot")->GetSubTable("position");
	
// }

VisionServer::TargetInfo::TargetInfo(const std::shared_ptr<nt::NetworkTable> table) : ttable(table) {}

void VisionServer::TargetInfo::setTarget(const std::string& target) {
	this->ttable->GetEntry("Active Target").SetString(target);
	this->last = CHRONO::high_resolution_clock::now();
}
bool VisionServer::TargetInfo::update(double ltime) {
	if(CHRONO::duration<double>(CHRONO::high_resolution_clock::now() - this->last).count() > ltime) {
		this->ttable->GetEntry("Active Target").SetString("none");
		return true;
	}
	return false;
}