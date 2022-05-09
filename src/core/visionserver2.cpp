#include "visionserver2.h"


using namespace vs2;

void VisionServer::BasePipe::setCamera(const VisionCamera& cam) {
	this->SetVideoMode(cam.GetVideoMode());
	this->input.SetSource(cam);
}


VisionServer::VisionServer() {
	// add networktable entries
}
VisionServer::~VisionServer() {
	// delete networktable entries
}

void VisionServer::addCamera(VisionCamera&& c) {
	if(!inst().is_running) {
		inst().cameras.emplace_back(std::move(c));
		inst().cameras.back().setNetworkBase(VisionServer::ntable);
		inst().cameras.back().setNetworkAdjustable();
	}
}
void VisionServer::addCameras(std::vector<VisionCamera>&& cms) {
	if(!inst().is_running) {
		size_t num = cms.size();
		inst().cameras.insert(
			inst().cameras.end(),
			std::make_move_iterator(cms.begin()),
			std::make_move_iterator(cms.end())
		);
		cms.clear();
		for(int i = ((int)inst().cameras.size()) - num; i < inst().cameras.size(); i++) {
			inst().cameras.at(i).setNetworkBase(VisionServer::ntable);
			inst().cameras.at(i).setNetworkAdjustable();
		}
	}
}
void VisionServer::setCameras(std::vector<VisionCamera>&& cms) {
	if(!inst().is_running) {
		inst().cameras = std::move(cms);
		cms.clear();
		for(size_t i = 0; i < inst().cameras.size(); i++) {
			inst().cameras.at(i).setNetworkBase(VisionServer::ntable);
			inst().cameras.at(i).setNetworkAdjustable();
		}
	}
}
void VisionServer::addStream() {
	if(!inst().is_running) {
		inst().streams.emplace_back(frc::CameraServer::AddServer("Stream " + std::to_string(inst().streams.size() + 1)));
	}
}
void VisionServer::addStream(std::string_view name) {
	if(!inst().is_running) {
		inst().streams.emplace_back(frc::CameraServer::AddServer(name));
	}
}
void VisionServer::addStream(std::string_view name, int port) {
	if(!inst().is_running) {
		inst().streams.emplace_back(frc::CameraServer::AddServer(name, port));
	}
}
void VisionServer::addStreams(size_t n) {
	if(!inst().is_running) {
		for(size_t i = 0; i < n; i++) {
			inst().streams.emplace_back(frc::CameraServer::AddServer("Stream " + std::to_string(inst().streams.size() + 1)));
		}
	}
}


void VisionServer::pipelineRunner(BasePipe* pipe, uint16_t rps) {
	pipe->table->PutBoolean("Enable Processing", true);
	pipe->table->PutNumber("Source Index", 1);
	pipe->table->PutNumber("Statistics Verbosity", 0);

	std::shared_ptr<nt::NetworkTable> stats = pipe->table->GetSubTable("stats");
	stats->PutNumber("RPS: ", rps);	// maybe put in VS not in each pipeline?

	int idx = 1;
	float max_ftime = 1.f / rps, util_percent = 0.f, fps = 0.f;
	double init_time = 0, proc_time = 0, out_time = 0, active_time = 1, full_time = 0;
	std::chrono::high_resolution_clock::time_point beg_frame, beg_proc, end_proc, end_frame, last;
	cv::Mat frame = cv::Mat::zeros(cv::Size(1, 1), CV_8UC3);

	while(inst().is_running) {
		beg_frame = std::chrono::high_resolution_clock::now();
		int n = pipe->table->GetEntry("Source Index").GetDouble(0);
		if(n != idx) {
			idx = n;
			if(idx > 0 && idx <= inst().cameras.size()) {
				pipe->setCamera(inst().cameras.at(idx - 1));
			} else if(idx < 0 && idx >= -((int)inst().pipelines.size())) {	// this does not work
				pipe->input.SetSource(*(inst().pipelines.at((-idx) - 1).get()));
			}
		}
		if(pipe->table->GetEntry("Enable Processing").GetBoolean(false) && 
			pipe->input.GrabFrame(frame/*, max_ftime / 2.f*/)
		) {
			beg_proc = std::chrono::high_resolution_clock::now();
			pipe->process(frame);
			end_proc = std::chrono::high_resolution_clock::now();
			int verbosity = pipe->table->GetEntry("Statistics Verbosity").GetDouble(0);
			if(verbosity > 0) {
				cv::putText(
					frame, std::to_string(fps),
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
					frame, "Output: " + std::to_string(out_time * 1000) + "ms",
					cv::Point(5, 145), cv::FONT_HERSHEY_DUPLEX,
					0.65, cv::Scalar(0, 255, 0), 1, cv::LINE_AA
				);
			}
			pipe->PutFrame(frame);
			end_frame = std::chrono::high_resolution_clock::now();
		} else {
			beg_proc = end_proc = end_frame = std::chrono::high_resolution_clock::now();
		}

		init_time = std::chrono::duration<double>(beg_proc - beg_frame).count();
		proc_time = std::chrono::duration<double>(end_proc - beg_proc).count();
		out_time = std::chrono::duration<double>(end_frame - end_proc).count();
		full_time = std::chrono::duration<double>(beg_frame - last).count();
		active_time = init_time + proc_time + out_time;
		util_percent = active_time / max_ftime * 100.f;
		fps = 1.f/full_time;

		stats->PutNumber("FPS: ", fps);
		stats->PutNumber("Utilization(%): ", util_percent);
		stats->PutNumber("Active time(ms): ", active_time * 1000);
		stats->PutNumber("Init time(ms): ", init_time * 1000);
		stats->PutNumber("Process time(ms): ", proc_time * 1000);
		stats->PutNumber("Output time(ms): ", out_time * 1000);

		std::this_thread::sleep_for(
			std::chrono::nanoseconds((uint64_t)(max_ftime * 1000000000)) - (
				std::chrono::high_resolution_clock::now() - beg_frame
			)
		);

		last = beg_frame;

	}
	pipe->table->Delete("Enable Processing");
	pipe->table->Delete("Source Index");
	pipe->table->Delete("Statistics Verbosity");

}



VisionServer::OutputStream::OutputStream(cs::MjpegServer&& s) :
	server(s), table(streams_table->GetSubTable(this->server.GetName()))
{
	//frc::CameraServer::AddServer(this->server);
	this->table->PutNumber("Source Index", -1);
	// add compression and other settings w/ callbacks...?
	this->table->PutNumber("Port", this->server.GetPort());
	this->table->GetEntry("Source Index").AddListener(	// negative -> cameras, positive -> pipelines, 0 -> nothing
		[this](const nt::EntryNotification& event) {
			if(event.value->IsDouble() && inst().is_running) {
				int idx = event.value->GetDouble();
				//std::cout << "Changing [" << this->server.GetName() << "] to source: " << idx << '\n';
				if(idx < 0 && idx >= -(int)inst().cameras.size()) {
					this->server.SetSource(inst().cameras.at((-idx) - 1));
				} else if(idx > 0 && idx <= (int)inst().pipelines.size()) {
					this->server.SetSource(*(inst().pipelines.at(idx - 1).get()));
				}
			}
		},
		NT_NOTIFY_IMMEDIATE | NT_NOTIFY_NEW | NT_NOTIFY_UPDATE
	);
}



bool VisionServer::run(uint16_t rps) {
	if(!inst().is_running) {
		inst().is_running = true;

		std::vector<std::thread> runners;
		for(size_t i = 0; i < inst().pipelines.size(); i++) {
			runners.emplace_back(std::thread(pipelineRunner, inst().pipelines.at(i).get(), rps));
		}

		std::chrono::high_resolution_clock::time_point tbuff;
		float max_millis = 1000.f / rps;

		while(inst().is_running) {	// main loop
			tbuff = std::chrono::high_resolution_clock::now();
			// update system stats?
			nt::NetworkTableInstance::GetDefault().Flush();
			// do work...

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
bool VisionServer::runThread(uint16_t rps) {
	if(!inst().is_running && !inst().head.joinable()) {
		inst().head = std::move(std::thread(VisionServer::run, rps));
		return true;
	}
	return false;
}
bool VisionServer::stop() {
	inst().is_running = false;
	if(inst().head.joinable()) {
		inst().head.join();
		return true;
	}
	return false;
}



void VisionServer::test() {
	if(
		inst().cameras.size() > 0 &&
		inst().pipelines.size() > 0 &&
		inst().streams.size() > 0
	) {
		inst().is_running = true;
		inst().pipelines.at(0).get()->input.SetSource(inst().cameras.at(1));
		inst().streams.at(0).server.SetSource(*inst().pipelines.at(0).get());
		VisionServer::pipelineRunner(inst().pipelines.at(0).get(), 30);
		inst().streams.at(0).server.SetSource(inst().cameras.at(1));
		for(;;) {
			std::this_thread::sleep_for(std::chrono::seconds(5));
		}
	} else {
		std::cout << "Test failed: resources not allocated.\n";
	}
}