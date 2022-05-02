#include "visionserver2.h"


VisionServer2::VisionServer2() {

}
VisionServer2::~VisionServer2() {
	
}

void VisionServer2::addCamera(VisionCamera&& c) {
	if(!inst().is_running) {
		inst().cameras.emplace_back(std::move(c));
		inst().cameras.back().setNetworkBase(VisionServer2::ntable);
		inst().cameras.back().setNetworkAdjustable();
	}
}
void VisionServer2::addCameras(std::vector<VisionCamera>&& cms) {
	if(!inst().is_running) {
		size_t num = cms.size();
		inst().cameras.insert(
			inst().cameras.end(),
			std::make_move_iterator(cms.begin()),
			std::make_move_iterator(cms.end())
		);
		cms.clear();
		for(int i = ((int)inst().cameras.size()) - num; i < inst().cameras.size(); i++) {
			inst().cameras.at(i).setNetworkBase(VisionServer2::ntable);
			inst().cameras.at(i).setNetworkAdjustable();
		}
	}
}
void VisionServer2::setCameras(std::vector<VisionCamera>&& cms) {
	if(!inst().is_running) {
		inst().cameras = std::move(cms);
		cms.clear();
		for(size_t i = 0; i < inst().cameras.size(); i++) {
			inst().cameras.at(i).setNetworkBase(VisionServer2::ntable);
			inst().cameras.at(i).setNetworkAdjustable();
		}
	}
}
// void VisionServer2::addPipeline(BasePipe* pipe) {	// this is likely to not work
// 	if(!inst().is_running) {
// 		inst().pipelines.emplace_back(
// 			std::unique_ptr<BasePipe, decltype(VisionServer2::nullPipeDeleter)>(pipe, VisionServer2::nullPipeDeleter)
// 		);
// 	}
// }
// void VisionServer2::addPipelines(std::initializer_list<BasePipe*> pipes) {
// 	// ????
// }
// void VisionServer2::setPipelines(std::initializer_list<BasePipe*> pipes) {
// 	// ???
// }
void VisionServer2::addStream(std::string_view name, int port) {
	addStream(cs::MjpegServer(name, port));
}
void VisionServer2::addStream(cs::MjpegServer&& server) {
	if(!inst().is_running) {
		inst().streams.emplace_back(std::move(server));
	}
}
void VisionServer2::addStreams(size_t n) {
	if(!inst().is_running) {
		for(size_t i = 0; i < n; i++) {
			addStream();
		}
	}
}


void VisionServer2::pipelineRunner(BasePipe* pipe, uint16_t rps) {
	pipe->table->PutBoolean("Enable Processing", true);
	pipe->table->PutNumber("Source Index", 0);
	pipe->table->PutNumber("Statistics Verbosity", 0);

	std::shared_ptr<nt::NetworkTable> stats = pipe->table->GetSubTable("stats");
	stats->PutNumber("RPS: ", rps);	// maybe put in VS not in each pipeline?

	float max_ftime = 1.f / rps, util_percent = 0.f, fps_ex = 0.f;
	double init_time = 0, proc_time = 0, out_time = 0, active_time = 1;
	std::chrono::high_resolution_clock::time_point beg_frame, beg_proc, end_proc, end_frame;
	cv::Mat frame;

	while(inst().is_running) {
		beg_frame = std::chrono::high_resolution_clock::now();
		int idx = pipe->table->GetEntry("Enable Processing").GetBoolean(false) * 
			((int)pipe->table->GetEntry("Source Index").GetDouble(-1) + 1);
		if(idx > 0 && idx <= inst().cameras.size()) {
			inst().framebuffer.at(idx - 1).clone(frame);
			beg_proc = std::chrono::high_resolution_clock::now();
			pipe->process(frame);
			end_proc = std::chrono::high_resolution_clock::now();
			int verbosity = pipe->table->GetEntry("Statistics Verbosity").GetDouble(0);
			if(verbosity > 0) {
				cv::putText(
					frame, std::to_string(fps_ex),
					cv::Point(5, 20), cv::FONT_HERSHEY_DUPLEX,
					0.65, cv::Scalar(0, 255, 0), 1, cv::LINE_AA
				);
			}
			if(verbosity > 1) {
				cv::putText(
					frame, "Util%: " + std::to_string(util_percent),
					cv::Point(5, 45), cv::FONT_HERSHEY_DUPLEX,
					0.65, cv::Scalar(0, 255, 0), 1, cv::LINE_AA
				);
				cv::putText(
					frame, "Active: " + std::to_string(active_time * 1000) + "ms",
					cv::Point(5, 70), cv::FONT_HERSHEY_DUPLEX,
					0.65, cv::Scalar(0, 255, 0), 1, cv::LINE_AA
				);
			}
			if(verbosity > 2) {
				cv::putText(
					frame, "Init: " + std::to_string(init_time * 1000) + "ms",
					cv::Point(5, 95), cv::FONT_HERSHEY_DUPLEX,
					0.65, cv::Scalar(0, 255, 0), 1, cv::LINE_AA
				);
				cv::putText(
					frame, "Process: " + std::to_string(proc_time * 1000) + "ms",
					cv::Point(5, 120), cv::FONT_HERSHEY_DUPLEX,
					0.65, cv::Scalar(0, 255, 0), 1, cv::LINE_AA
				);
				cv::putText(
					frame, "Output" + std::to_string(out_time * 1000) + "ms",
					cv::Point(5, 145), cv::FONT_HERSHEY_DUPLEX,
					0.65, cv::Scalar(0, 255, 0), 1, cv::LINE_AA
				);
			}
			pipe->output.PutFrame(frame);
			end_frame = std::chrono::high_resolution_clock::now();
		} else {
			beg_proc = end_proc = end_frame = std::chrono::high_resolution_clock::now();
		}

		init_time = std::chrono::duration<double>(beg_proc - beg_frame).count();
		proc_time = std::chrono::duration<double>(end_proc - beg_proc).count();
		out_time = std::chrono::duration<double>(end_frame - end_proc).count();
		active_time = init_time + proc_time + out_time;
		util_percent = active_time / max_ftime * 100.f;
		fps_ex = 1.f/active_time;

		stats->PutNumber("Extrapolated FPS: ", fps_ex);
		stats->PutNumber("Utilization(%): ", util_percent);
		stats->PutNumber("Active time(ms): ", active_time * 1000);
		stats->PutNumber("Init time(ms): ", init_time * 1000);
		stats->PutNumber("Process time(ms): ", proc_time * 1000);
		stats->PutNumber("Output time(ms): ", out_time * 1000);

		std::this_thread::sleep_for(
			std::chrono::nanoseconds((uint64_t)(max_ftime * 1000000)) - (
				std::chrono::high_resolution_clock::now() - beg_frame
			)
		);

	}
	pipe->table->Delete("Enable Processing");
	pipe->table->Delete("Source Index");
	pipe->table->Delete("Statistics Verbosity");

}

bool VisionServer2::FrameMutex::update(const VisionCamera& cam, double timeo) {
	if(this->access.try_lock()) {
		cam.getFrame(this->buffer, timeo);
		this->access.unlock();
		return true;
	}
	return false;
}
void VisionServer2::FrameMutex::transfer(cv::Mat& frame) {
	this->access.lock();
	frame = this->buffer;
	this->access.unlock();
}
void VisionServer2::FrameMutex::clone(cv::Mat& frame) {
	this->access.lock();
	this->buffer.copyTo(frame);
	this->access.unlock();
}

VisionServer2::OutputStream::OutputStream(cs::MjpegServer&& s) :
	server(s), table(streams_table->GetSubTable(this->server.GetName()))
{
	frc::CameraServer::AddServer(this->server);
	this->table->PutNumber("Source Index", -1);
	// add compression and other settings w/ callbacks...?
	this->table->PutNumber("Port", this->server.GetPort());
	this->table->GetEntry("Source Index").AddListener(
		[this](const nt::EntryNotification& event) {
			if(event.value->IsDouble()) {
				int idx = event.value->GetDouble();
				if(idx < 0 && idx >= -(int)inst().cameras.size()) {
					this->server.SetSource(inst().cameras.at((-idx) - 1));	// this won't work
				} else if(idx >= 0 && idx < (int)inst().pipelines.size()) {
					inst().pipelines.at(idx)->sendOutputTo(this->server);
				}
			}
		},
		NT_NOTIFY_IMMEDIATE | NT_NOTIFY_NEW | NT_NOTIFY_UPDATE
	);
}



bool VisionServer2::run(uint16_t rps) {
	if(!inst().is_running) {
		inst().is_running = true;

		std::vector<std::thread> runners;
		for(size_t i = 0; i < inst().pipelines.size(); i++) {
			runners.emplace_back(std::thread(pipelineRunner, inst().pipelines.at(i).get(), rps));
		}

		uint16_t max_fps = 1;
		std::chrono::high_resolution_clock::time_point tbuff;
		for(size_t i = 0; i < inst().cameras.size(); i++) {
			int fps = inst().cameras.at(i).getConfigFPS();
			max_fps = fps > max_fps ? fps : max_fps;
		}
		float max_millis = 1000.f / max_fps;
		for(size_t i = inst().framebuffer.size(); i < inst().cameras.size(); i++) {
			inst().framebuffer.emplace_back(inst().cameras.at(i).getResolution(), CV_8UC3);
		}
		double timeout = (max_millis - 5.f) / inst().cameras.size() / 1000.0;

		while(inst().is_running) {	// main loop
			tbuff = std::chrono::high_resolution_clock::now();
			for(size_t i = 0; i < inst().cameras.size(); i++) {	//????????
				std::cout << inst().framebuffer.size() << " : " << inst().cameras.size() << '\n';
				std::cout << inst().framebuffer[i].update(inst().cameras[i], timeout) << '\n';
			}
			// update system stats?
			nt::NetworkTableInstance::GetDefault().Flush();

			std::this_thread::sleep_for(
				std::chrono::nanoseconds((uint64_t)(max_millis * 1000000)) - (
					std::chrono::high_resolution_clock::now() - tbuff
				)
			);
		}

		for(size_t i = 0; i < runners.size(); i++) {
			runners.at(i).join();
		}
		return true;
	}
	return false;
}
bool VisionServer2::runThread(uint16_t rps) {
	if(!inst().is_running && !inst().head.joinable()) {
		inst().head = std::move(std::thread(VisionServer2::run, rps));
		return true;
	}
	return false;
}
bool VisionServer2::stop() {
	inst().is_running = false;
	if(inst().head.joinable()) {
		inst().head.join();
		return true;
	}
	return false;
}