#include "visionserver.h"

#include <networktables/EntryListenerFlags.h>
#include <networktables/NetworkTableEntry.h>
#include <wpi/raw_istream.h>
#include <wpi/raw_ostream.h>

#include "tools/src/unix/stats.h"
//#include "pipelines.h"
#include "vision.h"

VPipeline::VPipeline(VisionServer& server) : 
	name("Unnamed Pipeline"), table(server.vision->GetSubTable("Pipelines")->GetSubTable(this->name)), env(&server) {}
VPipeline::VPipeline(VisionServer& server, const char* name) : 
	name(name), table(server.vision->GetSubTable("Pipelines")->GetSubTable(name)), env(&server) {}
VPipeline::VPipeline(VisionServer& server, const std::string& name) : 
	name(name), table(server.vision->GetSubTable("Pipelines")->GetSubTable(name)), env(&server) {}
VPipeline::VPipeline(VPipeline&& other) : table(std::move(other.table)), env(other.env) {}
VPipeline::~VPipeline() {}
// VPipeline& VPipeline::operator=(VPipeline&& other) {
// 	this->table = std::move(other.table);
// 	this->env = other.env;
// 	return *this;
// }

const cv::Mat_<float>& VPipeline::getCameraMatrix() const {
	return this->env->getCameraMatrix();
}
const cv::Mat_<float>& VPipeline::getCameraDistortion() const {
	return this->env->getDistortion();
}
void VPipeline::updateTarget(const std::string& target) {
	this->env->updateTarget(target);
}
VisionCamera& VPipeline::getCurrentCamera() {
	return this->env->getCurrentCamera();
}
const std::string& VPipeline::getName() const {
	return this->name;
}
const std::shared_ptr<nt::NetworkTable> VPipeline::getTable() const {
	return this->table;
}
const VisionServer* VPipeline::getEnv() const {
	return this->env;
}

DefaultPipeline::DefaultPipeline(VisionServer& server) : VPipeline(server, "Default Pipeline") {}
// DefaultPipeline::DefaultPipeline(DefaultPipeline&& other) : VPipeline(static_cast<VPipeline&&>(other)) {}
// DefaultPipeline& DefaultPipeline::operator=(DefaultPipeline&& other) {
// 	this->table = std::move(other.table);
// 	this->env = other.env;
// 	return *this;
// }



VisionServer::VisionServer(const char* file) {
	this->updateFromConfig(file);

	this->vision->PutNumber("Camera Index", 0);
	this->vision->PutString("Camera Name", this->cameras[0].GetName());
	//this->vision->PutNumber("Cameras Available", cameras.size());

	this->source = this->cameras[0].getVideo();
	this->output = cs::CvSource("Vision Stream", this->cameras[0].GetVideoMode());
	this->stream = frc::CameraServer::AddServer("Vision Output");
	this->stream.SetSource(this->output);
	
	this->vision->GetEntry("Camera Index").AddListener(
		[this](const nt::EntryNotification& event) {
			if(event.value->IsDouble()) {
				size_t idx = event.value->GetDouble();
				if(idx >= 0 && idx < this->cameras.size()) {
					this->source.SetSource(cameras[idx]);
					this->vision->GetEntry("Camera Name").SetString(this->cameras[idx].GetName());
					this->cameras[idx].getCameraMatrix(this->camera_matrix);
					this->cameras[idx].getDistortion(this->distortion);
				}
			}
		},
		NT_NOTIFY_IMMEDIATE | NT_NOTIFY_NEW | NT_NOTIFY_UPDATE
	);
}
VisionServer::VisionServer(std::vector<VisionCamera>&& cameras) : cameras(std::move(cameras)) {
	for(size_t i = 0; i < this->cameras.size(); i++) {
		this->cameras[i].setNetworkBase(this->vision);
		this->cameras[i].setNetworkAdjustable();
	}
	this->vision->PutNumber("Camera Index", 0);
	this->vision->PutString("Camera Name", this->cameras[0].GetName());
	this->vision->PutNumber("Cameras Available", this->cameras.size());

	this->source = this->cameras[0].getVideo();
	this->output = cs::CvSource("Vision Stream", this->cameras[0].GetVideoMode());
	this->stream = frc::CameraServer::AddServer("Vision Output");
	this->stream.SetSource(this->output);
	
	this->vision->GetEntry("Camera Index").AddListener(
		[this](const nt::EntryNotification& event) {
			if(event.value->IsDouble()) {
				size_t idx = event.value->GetDouble();
				if(idx >= 0 && idx < this->cameras.size()) {
					this->source.SetSource(this->cameras[idx]);
					this->vision->GetEntry("Camera Name").SetString(this->cameras[idx].GetName());
					this->cameras[idx].getCameraMatrix(this->camera_matrix);
					this->cameras[idx].getDistortion(this->distortion);
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
VisionServer::~VisionServer() {
	this->vision->Delete("Camera Index");
	this->vision->Delete("Camera Name");
	this->vision->Delete("Cameras Available");
}

bool VisionServer::updateFromConfig(const char* file) {
	std::error_code ec;
    wpi::raw_fd_istream is(file, ec);
    if (ec) {
        wpi::errs() << "Could not open '" << file << "': " << ec.message() << newline;
        return false;
    }
    wpi::json j;
    try { j = wpi::json::parse(is); }
    catch (const wpi::json::parse_error& e) {
        wpi::errs() << "Config error in " << file << /*": byte " << e.byte <<*/ ": " << e.what() << newline;
        return false;
    }
    if (!j.is_object()) {
        wpi::errs() << "Config error in " << file << ": must be JSON object\n";
        return false;
    }
	if(j.count("ntmode") != 0 && !nt::NetworkTableInstance::GetDefault().IsConnected()) {
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
		std::vector<VisionCamera> backup(std::move(this->cameras));
		this->cameras.clear();
		try {
            for(const wpi::json& camera : j.at("cameras")) {
                if(camera.count("calibration") > 0 && j.count("calibrations") > 0) {
                    wpi::json calibration;
                    try {
                        calibration = j.at("calibrations").at(camera.at("calibration").get<std::string>()); 
                        this->cameras.emplace_back(camera, calibration);
                    } catch (const wpi::json::exception& e) {
                        wpi::errs() << "Config error in " << file << ": failed to get configuration object: " << e.what() << newline;  // print out more info if needed
                        this->cameras.emplace_back(camera);
                    }
                } else { this->cameras.emplace_back(camera); }
#ifdef REMOVE_DISCONNECTED_CAMERAS
                if(!this->cameras.back().IsConnected()) { this->cameras.pop_back(); } else
#endif
				{
					this->cameras.back().setNetworkBase(this->vision);
					this->cameras.back().setNetworkAdjustable();
				}
        	} 
			this->vision->PutNumber("Cameras Available", this->cameras.size());
		}
        catch (const wpi::json::exception& e) { 
			wpi::errs() << "Config error in " << file << ": could not read cameras: " << e.what() << newline; 
			this->cameras = std::move(backup);
		}
    }
	return true;
}

size_t VisionServer::validIndexes() const {	// make inline
    return this->cameras.size();
}
bool VisionServer::setCamera(size_t idx) {
    if(idx < this->validIndexes()) {
        //this->source.SetSource(this->cameras->operator[](idx));   // this should be taken care of by the networktables callback
        this->vision->GetEntry("Camera Index").SetDouble(idx);
    }
    return false;
}
cv::Size VisionServer::getCurrentResolution() const {
	return getResolution(this->source.GetSource().GetVideoMode());
}
void VisionServer::setCompression(int8_t quality) {
	this->stream.SetCompression(quality > 100 ? 100 : (quality < -1 ? -1 : quality));
	// add SetDefualtCompression as well?
}
const std::vector<VisionCamera>& VisionServer::getCameras() {
	return this->cameras;
}

bool VisionServer::stopVision() {
	this->runloop = false;
	if(this->launched.joinable()) {
		this->launched.join();
		return true;
	}
	return false;
}
bool VisionServer::runVisionThread(int8_t quality) {
	if(!this->launched.joinable()) {
		this->launched = std::move(std::thread(static_cast<void(*)(VisionServer&, int8_t)>(VisionServer::visionWorker), std::ref(*this), quality));
		return true;
	}
	return false;
}

void VisionServer::visionWorker(VisionServer& server, int8_t quality) {

	server.stream.SetCompression(quality);	// this should call the VisionServer method directly instead of from MjpegServer
	server.stream.SetDefaultCompression(quality);
	server.vision->PutBoolean("Show Statistics", false);

	cv::Mat frame(server.getCurrentResolution(), CV_8UC3);
	while(server.runloop) {
		server.source.GrabFrame(frame);
		server.beg = server.end = CHRONO::high_resolution_clock::now();
		server.total_frames++;

		server.total_time = CHRONO::duration<double>(server.end - server.start).count();
		server.frame_time = CHRONO::duration<double>(server.end - server.beg).count();
		server.loop_time = CHRONO::duration<double>(server.end - server.last).count();
		server.last = server.end;

		server.active_target.update(server.loop_time);

		server.fps = 1.f/server.loop_time;
		if((int)server.total_time > (int)server.sec1_time) {
			server.fps_1s = ((server.total_frames - server.sec1_frames) / (server.total_time - server.sec1_time));
			server.sec1_time = server.total_time;
			server.sec1_frames = server.total_frames;
		}

		if(server.vision->GetEntry("Show Statistics").GetBoolean(false)) {
			server.putStats(frame);
		}
		server.updateStats();
		server.output.PutFrame(frame);
	}
	server.vision->Delete("Show Statistics");
	server.runloop = true;
}

void VisionServer::putStats(cv::Mat& io_frame) {
	cv::putText(
		io_frame, "FPS(1F): " + std::to_string(this->fps), 
		cv::Point(5, 20), 
		cv::FONT_HERSHEY_DUPLEX, 0.65, cv::Scalar(0, 255, 0), 1, cv::LINE_AA
	);
	cv::putText(
		io_frame, "FPS(1S): " + std::to_string(this->fps_1s), 
		cv::Point(5, 44), 
		cv::FONT_HERSHEY_DUPLEX, 0.65, cv::Scalar(0, 255, 0), 1, cv::LINE_AA
	);
	cv::putText(
		io_frame, "Ltime: " + std::to_string(this->loop_time*1000) + "ms", 
		cv::Point(5, 70), 
		cv::FONT_HERSHEY_DUPLEX, 0.65, cv::Scalar(0, 255, 0), 1, cv::LINE_AA
	);
	cv::putText(
		io_frame, "Ftime: " + std::to_string(this->frame_time*1000) + "ms", 
		cv::Point(5, 95), 
		cv::FONT_HERSHEY_DUPLEX, 0.65, cv::Scalar(0, 255, 0), 1, cv::LINE_AA
	);
}
void VisionServer::updateStats() {
	std::shared_ptr<nt::NetworkTable> stats = this->vision->GetSubTable("stats");
	stats->PutNumber("CPU Utilization(%-1F)", CPU::get().refPercent()*100.f);
	stats->PutNumber("CPU Temp(*C)", CPU::temp());
	stats->PutNumber("Frametime(ms)", this->frame_time*1000);
	stats->PutNumber("Looptime(ms)", this->loop_time*1000);
	stats->PutNumber("FPS(1F)", this->fps);
	stats->PutNumber("FPS(1s)", this->fps_1s);
	stats->PutNumber("Frames", this->total_frames);
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
VisionCamera& VisionServer::getCurrentCamera() {
	return this->cameras.at(this->vision->GetEntry("Camera Index").GetDouble(0));
}

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