#include "visionserver2.h"


using namespace vs2;

void VisionServer::BasePipe::setCamera(const VisionCamera& cam) {
	this->SetVideoMode(cam.GetVideoMode());
	this->input.SetSource(cam);
	this->src_matrix = &cam.getCameraMatrix();
	this->src_distort = &cam.getDistortionCoefs();
}
void VisionServer::BasePipe::setPipeline(const BasePipe& pipe) {
	this->SetVideoMode(pipe.GetVideoMode());
	this->input.SetSource(pipe);
	this->src_matrix = pipe.src_matrix;
	this->src_distort = pipe.src_distort;
}
void VisionServer::BasePipe::setSource(const cs::VideoSource& src) {
	this->SetVideoMode(src.GetVideoMode());
	this->input.SetSource(src);
	this->src_matrix = &VisionCamera::default_matrix;
	this->src_distort = &VisionCamera::default_distort;
}


VisionServer::VisionServer() :
	base(nt::NetworkTableInstance::GetDefault().GetTable("VisionServer")),
	pipes(nt::NetworkTableInstance::GetDefault().GetTable("VisionServer")->GetSubTable("Pipelines")),
	outputs(nt::NetworkTableInstance::GetDefault().GetTable("VisionServer")->GetSubTable("Streams")),
	targets(nt::NetworkTableInstance::GetDefault().GetTable("VisionServer")->GetSubTable("Targets"))
{
	std::cout << "Init VisionServer" << std::endl;
	this->base->PutNumber("Cameras Available", 0);
	this->base->PutNumber("Pipelines Available", 0);
	this->base->PutString("Status", "Offline");
}
VisionServer::~VisionServer() {
	this->base->Delete("Cameras Available");
	this->base->Delete("Pipelines Available");
	this->base->Delete("Status");
}

void VisionServer::addCamera(VisionCamera&& c) {
	if(!inst().is_running) {
		inst().cameras.emplace_back(std::move(c));
		inst().cameras.back().setNetworkBase(VisionServer::getTable());
		inst().cameras.back().setNetworkAdjustable();
		VisionServer::getTable()->GetEntry("Cameras Available").SetDouble(inst().cameras.size());
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
			inst().cameras.at(i).setNetworkBase(VisionServer::getTable());
			inst().cameras.at(i).setNetworkAdjustable();
		}
		VisionServer::getTable()->GetEntry("Cameras Available").SetDouble(inst().cameras.size());
	}
}
void VisionServer::setCameras(std::vector<VisionCamera>&& cms) {
	if(!inst().is_running) {
		inst().cameras = std::move(cms);
		cms.clear();
		for(size_t i = 0; i < inst().cameras.size(); i++) {
			inst().cameras.at(i).setNetworkBase(VisionServer::getTable());
			inst().cameras.at(i).setNetworkAdjustable();
		}
		VisionServer::getTable()->GetEntry("Cameras Available").SetDouble(inst().cameras.size());
	}
}
void VisionServer::addPipeline(BasePipe* p) {
	if(!inst().is_running) {
		inst().pipelines.push_back(p);
		VisionServer::getTable()->GetEntry("Pipelines Available").SetDouble(inst().pipelines.size());
	}
}
void VisionServer::addPipelines(std::vector<BasePipe*>&& ps) {
	if(!inst().is_running) {
		inst().pipelines.insert(inst().pipelines.end(), ps.begin(), ps.end());
		VisionServer::getTable()->GetEntry("Pipelines Available").SetDouble(inst().pipelines.size());
	}
}
void VisionServer::addPipelines(std::initializer_list<BasePipe*> ps) {
	if(!inst().is_running) {
		inst().pipelines.insert(inst().pipelines.end(), ps.begin(), ps.end());
		VisionServer::getTable()->GetEntry("Pipelines Available").SetDouble(inst().pipelines.size());
	}
}
void VisionServer::setPipelines(std::vector<BasePipe*>&& ps) {
	if(!inst().is_running) {
		inst().pipelines.clear();
		inst().heap_allocated.clear();
		inst().pipelines = std::move(ps);
		VisionServer::getTable()->GetEntry("Pipelines Available").SetDouble(inst().pipelines.size());
	}
}
void VisionServer::setPipelines(std::initializer_list<BasePipe*> ps) {
	if(!inst().is_running) {
		inst().pipelines.clear();
		inst().heap_allocated.clear();
		inst().pipelines = ps;
		VisionServer::getTable()->GetEntry("Pipelines Available").SetDouble(inst().pipelines.size());
	}
}
void VisionServer::addStream() {
	if(!inst().is_running) {
		inst().streams.emplace_back("Stream " + std::to_string(inst().streams.size() + 1));
	}
}
void VisionServer::addStream(std::string_view name) {
	if(!inst().is_running) {
		inst().streams.emplace_back(name);
	}
}
void VisionServer::addStream(std::string_view name, int port) {
	if(!inst().is_running) {
		inst().streams.emplace_back(name, port);
	}
}
void VisionServer::addStream(const cs::MjpegServer& s) {
	if(!inst().is_running) {
		inst().streams.emplace_back(cs::MjpegServer(s));
	}
}
void VisionServer::addStream(cs::MjpegServer&& s) {
	if(!inst().is_running) {
		inst().streams.emplace_back(std::move(s));
	}
}
void VisionServer::addStreams(size_t n) {
	if(!inst().is_running) {
		for(size_t i = 0; i < n; i++) {
			inst().streams.emplace_back("Stream " + std::to_string(inst().streams.size() + 1));
		}
	}
}
void VisionServer::addStreams(std::initializer_list<std::string_view> strms) {
	if(!inst().is_running) {
		for(auto itr = strms.begin(); itr != strms.end(); itr++) {
			inst().streams.emplace_back(*itr);
		}
	}
}
void VisionServer::addStreams(std::initializer_list<std::pair<std::string_view, int> > strms) {
	if(!inst().is_running) {
		for(auto itr = strms.begin(); itr != strms.end(); itr++) {
			inst().streams.emplace_back(itr->first, itr->second);
		}
	}
}
void VisionServer::addStreams(const std::vector<cs::MjpegServer>& strms) {
	if(!inst().is_running) {
		for(size_t i = 0; i < strms.size(); i++) {
			inst().streams.push_back(cs::MjpegServer(strms.at(i)));
		}
	}
}


void VisionServer::pipelineRunner(BasePipe* pipe, uint16_t rps) {
	pipe->table->PutBoolean("Enable Processing", true);
	pipe->table->PutNumber("Source Index", 1);
	pipe->table->PutNumber("Statistics Verbosity", 0);

	std::shared_ptr<nt::NetworkTable> stats = pipe->table->GetSubTable("stats");

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
			} else if(idx < 0 && idx >= -((int)inst().pipelines.size())) {
				pipe->setPipeline(*(inst().pipelines.at((-idx) - 1)));
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
			std::chrono::nanoseconds((uint64_t)(max_ftime * 1E9)) - (
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
	MjpegServer(std::move(s)), table(streamsTable()->GetSubTable(this->GetName()))
{
	this->table->PutNumber("Source Index", this->local_idx);
	this->table->PutNumber("Port", this->GetPort());
	std::cout << "OutputStream created: " << this->GetName()  << " - " << this->GetPort() << std::endl;

	// this->listener = this->table->GetEntry("Source Index").AddListener(	// negative -> cameras, positive -> pipelines, 0 -> nothing
	// 	[&](const nt::EntryNotification& event) {
	// 		if(event.value->IsDouble()) {
	// 			int idx = event.value->GetDouble();
	// 			std::cout << "Changing [" << this->GetName() << "] to source: " << idx << std::endl;
	// 			if(idx < 0 && idx >= -(int)inst().cameras.size()) {
	// 				this->SetSource(inst().cameras.at((-idx) - 1));
	// 			} else if(idx > 0 && idx <= (int)inst().pipelines.size()) {
	// 				this->SetSource(*(inst().pipelines.at(idx - 1)));
	// 			}
	// 		}
	// 	},
	// 	NT_NOTIFY_IMMEDIATE | NT_NOTIFY_UPDATE | NT_NOTIFY_LOCAL
	// );
}
VisionServer::OutputStream::~OutputStream() {
	//nt::RemoveEntryListener(this->listener);
	this->table->Delete("Source Index");
	this->table->Delete("Port");
}
void VisionServer::OutputStream::setSourceIdx(int i) {
	if(this->local_idx != i) {
		if(i < 0 && i >= -(int)inst().cameras.size()) {
			this->SetSource(inst().cameras.at((-i) - 1));
		} else if(i > 0 && i <= (int)inst().pipelines.size()) {
			this->SetSource(*(inst().pipelines.at(i - 1)));
		}
		this->table->GetEntry("Source Index").SetDouble(this->local_idx = i);
	}
}
void VisionServer::OutputStream::updateNt() {
	int nt = this->table->GetEntry("Source Index").GetDouble(0);
	if(nt != this->local_idx) {
		this->local_idx = nt;
		if(nt < 0 && nt >= -(int)inst().cameras.size()) {
			this->SetSource(inst().cameras.at((-nt) - 1));
		} else if(nt > 0 && nt <= (int)inst().pipelines.size()) {
			this->SetSource(*(inst().pipelines.at(nt - 1)));
		}
	}
}

void VisionServer::compensate() {
	VisionServer& _inst = inst();
	int16_t active = -1;
	for(size_t i = 0; i < _inst.cameras.size(); i++) {
		if(_inst.cameras.at(i).IsConnected()) {
			active = i;
			break;
		}
	}
	if(active + 1) {
		size_t i = 0;
		for(; i < _inst.pipelines.size(); i++) {
			std::cout << "Compensation - connected 1st camera to: " << _inst.pipelines[i]->getName() << std::endl;
			_inst.pipelines.at(i)->setCamera(_inst.cameras.at(active));
		}
		if(i) {
			for(size_t j = 0; j < _inst.streams.size(); j++) {
				std::cout << "Compensation - connected 1st pipeline to: " << _inst.streams[j].GetName() << std::endl;
				//_inst.streams.at(j).SetSource(*_inst.pipelines[1]);
				_inst.streams.at(j).setPipelineIdx(1);
			}
		} else {
			for(size_t j = 0; j < _inst.streams.size(); j++) {
				std::cout << "Compensation - connected 1st camera to: " << _inst.streams[i].GetName() << std::endl;
				//_inst.streams.at(j).SetSource(_inst.cameras[active]);
				_inst.streams.at(j).setCameraIdx(active + 1);
			}
		}
	} else {
		std::cout << "Compensation failed. No cameras available." << std::endl;
	}
}



bool VisionServer::run(uint16_t rps) {
	if(!inst().is_running) {
		inst().is_running = true;

		std::vector<std::thread> runners;
		for(size_t i = 0; i < inst().pipelines.size(); i++) {
			runners.emplace_back(std::thread(pipelineRunner, inst().pipelines.at(i), rps));
		}

		VisionServer::getTable()->GetEntry("Status").SetString("Running Multithreaded");
		VisionServer::getTable()->PutNumber("Target FPS", rps);

		std::chrono::high_resolution_clock::time_point tbuff;
		uint64_t max_nanos = 1E9 / rps;

		while(inst().is_running) {	// main loop
			tbuff = std::chrono::high_resolution_clock::now();
			// update system stats?
			nt::NetworkTableInstance::GetDefault().Flush();
			for(size_t i = 0; i < inst().streams.size(); i++) {
				inst().streams[i].updateNt();
			}

			std::this_thread::sleep_for(
				std::chrono::nanoseconds(max_nanos) - (std::chrono::high_resolution_clock::now() - tbuff)
			);
		}

		for(size_t i = 0; i < runners.size(); i++) {
			runners.at(i).join();
		}
		VisionServer::getTable()->GetEntry("Status").SetString("Offline");
		VisionServer::getTable()->Delete("Target FPS");
		return true;
	}
	return false;
}
bool VisionServer::runSingle(uint16_t rps) {
	if(!inst().is_running) {
		inst().is_running = true;

		VisionServer::getTable()->GetEntry("Status").SetString("Running Singlethreaded");
		VisionServer::getTable()->PutBoolean("Enable Processing", true);
		VisionServer::getTable()->PutNumber("Camera Index", 1);
		VisionServer::getTable()->PutNumber("Pipeline Index", 1);
		VisionServer::getTable()->PutNumber("Statistics Verbosity", 0);
		VisionServer::getTable()->PutNumber("Target FPS", rps);

		std::shared_ptr<nt::NetworkTable> stats = VisionServer::getTable()->GetSubTable("stats");

		int c_idx = 1, p_idx = 1;
		float max_ftime = 1.f / rps, util_percent = 0.f, fps = 0.f;
		double init_time = 0, proc_time = 0, out_time = 0, active_time = 1, full_time = 0;
		std::chrono::high_resolution_clock::time_point beg_frame, beg_proc, end_proc, end_frame, last;
		cv::Mat frame = cv::Mat::zeros(cv::Size(1, 1), CV_8UC3);

		while(inst().is_running) {
			beg_frame = std::chrono::high_resolution_clock::now();
			int n = VisionServer::getTable()->GetEntry("Pipeline Index").GetDouble(0);
			if(n != p_idx + 1 && n > 0 && n <= inst().pipelines.size()) {
				p_idx = n - 1;
				for(size_t i = 0; i < inst().streams.size(); i++) {
					inst().streams[i].setSourceIdx(n);
				}
			}
			n = VisionServer::getTable()->GetEntry("Camera Index").GetDouble(0);
			if(n != c_idx) {
				c_idx = n;
				if(c_idx > 0 && c_idx <= inst().cameras.size()) {
					inst().pipelines[p_idx]->setCamera(inst().cameras[c_idx]);
				}
			}
			for(size_t i = 0; i < inst().streams.size(); i++) {
				inst().streams[i].updateNt();
			}
			if(VisionServer::getTable()->GetEntry("Enable Processing").GetBoolean(false) &&
				inst().pipelines[p_idx]->input.GrabFrame(frame)
			) {
				beg_proc = std::chrono::high_resolution_clock::now();
				inst().pipelines[p_idx]->process(frame);
				end_proc = std::chrono::high_resolution_clock::now();
				int verbosity = VisionServer::getTable()->GetEntry("Statistics Verbosity").GetDouble(0);
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
				inst().pipelines[p_idx]->PutFrame(frame);
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

			nt::NetworkTableInstance::GetDefault().Flush();

			std::this_thread::sleep_for(
				std::chrono::nanoseconds((uint64_t)(max_ftime * 1E9)) - (
					std::chrono::high_resolution_clock::now() - beg_frame
				)
			);

			last = beg_frame;
		}
		VisionServer::getTable()->GetEntry("Status").SetString("Offline");
		VisionServer::getTable()->Delete("Enable Processing");
		VisionServer::getTable()->Delete("Camera Index");
		VisionServer::getTable()->Delete("Pipeline Index");
		VisionServer::getTable()->Delete("Statistics Verbosity");
		VisionServer::getTable()->Delete("Target FPS");

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
bool VisionServer::runSingleThread(uint16_t rps) {
	if(!inst().is_running && !inst().head.joinable()) {
		inst().head = std::move(std::thread(VisionServer::runSingle, rps));
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