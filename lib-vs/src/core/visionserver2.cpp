#include "visionserver2.h"

#include "cpp-tools/src/resources.h"


using namespace vs2;

VisionServer::VisionServer() {
	std::cout << "Init VisionServer" << std::endl;
	ntable()->PutNumber("Cameras Available", 0);
	ntable()->PutNumber("Pipelines Available", 0);
	ntable()->PutString("Status", "Offline");
}
VisionServer::~VisionServer() {
	ntable()->Delete("Cameras Available");
	ntable()->Delete("Pipelines Available");
	ntable()->Delete("Status");
}


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
void VisionServer::BasePipe::getFrame(cv::Mat& io_frame) {
	this->input.GrabFrame(io_frame);
}


bool VisionServer::addCamera(VisionCamera&& c) {
	if(!VisionServer::isRunning()) {
		VisionServer& _inst = getInstance();
		_inst.cameras.emplace_back(std::move(c));
		_inst.cameras.back().setNetworkBase(VisionServer::ntable());
		_inst.cameras.back().setNetworkAdjustable();
		VisionServer::ntable()->GetEntry("Cameras Available").SetDouble(_inst.cameras.size());
		return true;
	}
	return false;
}
bool VisionServer::addCameras(std::vector<VisionCamera>&& cms) {
	if(!VisionServer::isRunning()) {
		VisionServer& _inst = getInstance();
		size_t num = cms.size();
		_inst.cameras.reserve(_inst.cameras.size() + num);
		_inst.cameras.insert(
			_inst.cameras.end(),
			std::make_move_iterator(cms.begin()),
			std::make_move_iterator(cms.end())
		);
		cms.clear();
		for(size_t i = _inst.cameras.size() - num; i < _inst.cameras.size(); i++) {
			_inst.cameras.at(i).setNetworkBase(VisionServer::ntable());
			_inst.cameras.at(i).setNetworkAdjustable();
		}
		VisionServer::ntable()->GetEntry("Cameras Available").SetDouble(_inst.cameras.size());
		return true;
	}
	return false;
}
bool VisionServer::setCameras(std::vector<VisionCamera>&& cms) {
	if(!VisionServer::isRunning()) {
		VisionServer& _inst = getInstance();
		_inst.cameras = std::move(cms);
		cms.clear();
		for(size_t i = 0; i < _inst.cameras.size(); i++) {
			_inst.cameras.at(i).setNetworkBase(VisionServer::ntable());
			_inst.cameras.at(i).setNetworkAdjustable();
		}
		VisionServer::ntable()->GetEntry("Cameras Available").SetDouble(_inst.cameras.size());
		return true;
	}
	return false;
}

bool VisionServer::addPipeline(BasePipe* p) {
	if(!VisionServer::isRunning()) {
		VisionServer& _inst = getInstance();
		_inst.pipelines.push_back(p);
		VisionServer::ntable()->GetEntry("Pipelines Available").SetDouble(_inst.pipelines.size());
		return true;
	}
	return false;
}
bool VisionServer::addPipelines(std::vector<BasePipe*>&& ps) {
	if(!VisionServer::isRunning()) {
		VisionServer& _inst = getInstance();
		_inst.pipelines.reserve(_inst.pipelines.size() + ps.size());
		_inst.pipelines.insert(_inst.pipelines.end(), ps.begin(), ps.end());
		VisionServer::ntable()->GetEntry("Pipelines Available").SetDouble(_inst.pipelines.size());
		return true;
	}
	return false;
}
bool VisionServer::addPipelines(std::initializer_list<BasePipe*> ps) {
	if(!VisionServer::isRunning()) {
		VisionServer& _inst = getInstance();
		_inst.pipelines.reserve(_inst.pipelines.size() + ps.size());
		_inst.pipelines.insert(_inst.pipelines.end(), ps.begin(), ps.end());
		VisionServer::ntable()->GetEntry("Pipelines Available").SetDouble(_inst.pipelines.size());
		return true;
	}
	return false;
}
bool VisionServer::setPipelines(std::vector<BasePipe*>&& ps) {
	if(!VisionServer::isRunning()) {
		VisionServer& _inst = getInstance();
		_inst.pipelines.clear();
		_inst.heap_allocated.clear();
		_inst.pipelines = std::move(ps);
		VisionServer::ntable()->GetEntry("Pipelines Available").SetDouble(_inst.pipelines.size());
		return true;
	}
	return false;
}
bool VisionServer::setPipelines(std::initializer_list<BasePipe*> ps) {
	if(!VisionServer::isRunning()) {
		VisionServer& _inst = getInstance();
		_inst.pipelines.clear();
		_inst.heap_allocated.clear();
		_inst.pipelines = ps;
		VisionServer::ntable()->GetEntry("Pipelines Available").SetDouble(_inst.pipelines.size());
		return true;
	}
	return false;
}

bool VisionServer::addStream() {
	if(!VisionServer::isRunning()) {
		VisionServer& _inst = getInstance();
		_inst.streams.emplace_back("Stream " + std::to_string(_inst.streams.size() + 1));
		return true;
	}
	return false;
}
bool VisionServer::addStream(std::string_view name) {
	if(!VisionServer::isRunning()) {
		inst().streams.emplace_back(name);
		return true;
	}
	return false;
}
bool VisionServer::addStream(std::string_view name, int port) {
	if(!VisionServer::isRunning()) {
		inst().streams.emplace_back(name, port);
		return true;
	}
	return false;
}
bool VisionServer::addStream(const cs::MjpegServer& s) {
	if(!VisionServer::isRunning()) {
		inst().streams.emplace_back(cs::MjpegServer(s));
		return true;
	}
	return false;
}
bool VisionServer::addStream(cs::MjpegServer&& s) {
	if(!VisionServer::isRunning()) {
		inst().streams.emplace_back(std::move(s));
		return true;
	}
	return false;
}
bool VisionServer::addStreams(size_t n) {
	if(!VisionServer::isRunning()) {
		VisionServer& _inst = getInstance();
		_inst.streams.reserve(_inst.streams.size() + n);
		for(size_t i = 0; i < n; i++) {
			_inst.streams.emplace_back("Stream " + std::to_string(_inst.streams.size() + 1));
		}
		return true;
	}
	return false;
}
bool VisionServer::addStreams(std::initializer_list<std::string_view> strms) {
	if(!VisionServer::isRunning()) {
		VisionServer& _inst = getInstance();
		_inst.streams.reserve(_inst.streams.size() + strms.size());
		for(auto itr = strms.begin(); itr != strms.end(); itr++) {
			_inst.streams.emplace_back(*itr);
		}
		return true;
	}
	return false;
}
bool VisionServer::addStreams(std::initializer_list<std::pair<std::string_view, int> > strms) {
	if(!VisionServer::isRunning()) {
		VisionServer& _inst = getInstance();
		_inst.streams.reserve(_inst.streams.size() + strms.size());
		for(auto itr = strms.begin(); itr != strms.end(); itr++) {
			_inst.streams.emplace_back(itr->first, itr->second);
		}
		return true;
	}
	return false;
}
bool VisionServer::addStreams(const std::vector<cs::MjpegServer>& strms) {
	if(!VisionServer::isRunning()) {
		VisionServer& _inst = getInstance();
		_inst.streams.reserve(_inst.streams.size() + strms.size());
		for(size_t i = 0; i < strms.size(); i++) {
			_inst.streams.push_back(cs::MjpegServer(strms.at(i)));
		}
		return true;
	}
	return false;
}


VisionServer::OutputStream::OutputStream(cs::MjpegServer&& s) :
	MjpegServer(std::move(s)), table(OutputStream::ntable()->GetSubTable(this->GetName()))
{
	this->table->GetEntry("Source Index").SetDouble(this->local_idx);
	this->table->GetEntry("Port").SetDouble(this->GetPort());
}
VisionServer::OutputStream::~OutputStream() {}
void VisionServer::OutputStream::setSourceIdx(int i) {
	if(this->local_idx != i) {
		if(i < 0 && i >= -(int)VisionServer::numCameras()) {
			this->SetSource(inst().cameras.at((-i) - 1));
		} else if(i > 0 && i <= (int)VisionServer::numPipelines()) {
			this->SetSource(*(inst().pipelines.at(i - 1)));
		}
		this->table->GetEntry("Source Index").SetDouble(this->local_idx = i);
	}
}
void VisionServer::OutputStream::syncIdx() {
	int nt = this->table->GetEntry("Source Index").GetDouble(0);
	if(nt != this->local_idx) {
		this->local_idx = nt;
		if(nt < 0 && nt >= -(int)VisionServer::numCameras()) {
			this->SetSource(inst().cameras.at((-nt) - 1));
		} else if(nt > 0 && nt <= (int)VisionServer::numPipelines()) {
			this->SetSource(*(inst().pipelines.at(nt - 1)));
		}
	}
}


void VisionServer::pipelineRunner(BasePipe* pipe, uint16_t rps) {
	pipe->table->PutBoolean("Enable Processing", true);
	pipe->table->PutNumber("Source Index", 1);
	pipe->table->PutNumber("Statistics Verbosity", 0);

	VisionServer& _inst = VisionServer::getInstance();
	std::shared_ptr<nt::NetworkTable> stats{pipe->table->GetSubTable("stats")};

	int idx = 1;
	float max_ftime = 1.f / rps, util_percent = 0.f, fps = 0.f;
	double init_time = 0, proc_time = 0, out_time = 0, active_time = 1, full_time = 0;
	std::chrono::high_resolution_clock::time_point beg_frame, beg_proc, end_proc, end_frame, last;
	cv::Mat frame = cv::Mat::zeros(cv::Size(1, 1), CV_8UC3);

	while(VisionServer::isRunning()) {
		beg_frame = std::chrono::high_resolution_clock::now();
		int n = pipe->table->GetEntry("Source Index").GetDouble(0);
		if(n != idx) {
			idx = n;
			if(idx > 0 && idx <= numCameras()) {
				pipe->setCamera(_inst.cameras.at(idx - 1));
			} else if(idx < 0 && idx >= -((int)numPipelines)) {
				pipe->setPipeline(*(_inst.pipelines.at((-idx) - 1)));
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

bool VisionServer::compensate() {
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
			_inst.pipelines.at(i)->setCamera(_inst.cameras.at(active));
			std::cout << "Compensation: connected [pipeline] "
				<< _inst.pipelines[i]->getName() << " to [camera] "
				<< _inst.cameras[active].GetName() << newline;
		}
		if(i) {
			for(size_t j = 0; j < _inst.streams.size(); j++) {
				_inst.streams.at(j).setPipelineIdx(1);
				std::cout << "Compensation: connected [stream] "
					<< _inst.streams[j].GetName() << " to [pipeline] "
					<< _inst.pipelines[0]->getName() << newline;
			}
		} else {
			for(size_t j = 0; j < _inst.streams.size(); j++) {
				_inst.streams.at(j).setCameraIdx(active + 1);
				std::cout << "Compensation: connected [stream] "
					<< _inst.streams[i].GetName() << " to [camera] "
					<< _inst.cameras[active].GetName() << newline;
			}
		}
		std::cout.flush();
		return true;
	} else {
		std::cout << "Compensation failed. No cameras available." << std::endl;
		return false;
	}
}
bool VisionServer::runRaw() {
	if(VisionServer::isRunning()) {
		return false;
	}
	VisionServer& _inst = getInstance();
	while(!VisionServer::isRunning()) {
		for(size_t i = 0; i < _inst.streams.size(); i++) {
			_inst.streams.at(i).syncIdx();
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	return true;
}
bool VisionServer::runRawThread() {
	if(!VisionServer::isRunning()) {
		std::thread(VisionServer::runRaw).detach();
		return true;
	}
	return false;
}



bool VisionServer::run(uint16_t rps) {
	if(!VisionServer::isRunning()) {
		VisionServer& _inst = getInstance();
		_inst.is_running = true;

		std::vector<std::thread> runners;
		for(size_t i = 0; i < _inst.pipelines.size(); i++) {
			runners.emplace_back(std::thread(pipelineRunner, _inst.pipelines.at(i), rps));
		}

		VisionServer::ntable()->GetEntry("Status").SetString("Running Multithreaded");
		VisionServer::ntable()->PutNumber("Target FPS", rps);

		std::chrono::high_resolution_clock::time_point tbuff;
		uint64_t max_nanos = 1E9 / rps;

		while(isRunning()) {	// main loop
			tbuff = std::chrono::high_resolution_clock::now();
			// update system stats?
			nt::NetworkTableInstance::GetDefault().Flush();
			for(size_t i = 0; i < _inst.streams.size(); i++) {
				_inst.streams[i].syncIdx();
			}

			std::this_thread::sleep_for(
				std::chrono::nanoseconds(max_nanos) - (std::chrono::high_resolution_clock::now() - tbuff)
			);
		}

		for(size_t i = 0; i < runners.size(); i++) {
			runners.at(i).join();
		}
		VisionServer::ntable()->GetEntry("Status").SetString("Offline");
		VisionServer::ntable()->Delete("Target FPS");
		return true;
	}
	return false;
}
bool VisionServer::runSingle(uint16_t rps) {
	if(!VisionServer::isRunning()) {
		VisionServer& _inst = getInstance();
		_inst.is_running = true;

		VisionServer::ntable()->GetEntry("Status").SetString("Running Singlethreaded");
		VisionServer::ntable()->PutBoolean("Enable Processing", true);
		VisionServer::ntable()->PutNumber("Camera Index", 1);
		VisionServer::ntable()->PutNumber("Pipeline Index", 1);
		VisionServer::ntable()->PutNumber("Statistics Verbosity", 0);
		VisionServer::ntable()->PutNumber("Target FPS", rps);

		std::shared_ptr<nt::NetworkTable> stats = VisionServer::ntable()->GetSubTable("stats");

		int c_idx = 0, p_idx = 1;
		float max_ftime = 1.f / rps, util_percent = 0.f, fps = 0.f;
		double init_time = 0, proc_time = 0, out_time = 0, active_time = 1, full_time = 0;
		std::chrono::high_resolution_clock::time_point beg_frame, beg_proc, end_proc, end_frame, last;
		cv::Mat frame = cv::Mat::zeros(cv::Size(1, 1), CV_8UC3);

		while(isRunning()) {
			beg_frame = std::chrono::high_resolution_clock::now();
			int n = VisionServer::ntable()->GetEntry("Pipeline Index").GetDouble(0);
			if(n != p_idx + 1 && n > 0 && n <= numPipelines()) {
				p_idx = n - 1;
				for(size_t i = 0; i < _inst.streams.size(); i++) {
					_inst.streams[i].setSourceIdx(n);
				}
			}
			n = VisionServer::ntable()->GetEntry("Camera Index").GetDouble(0);
			if(n != c_idx) {
				c_idx = n;
				if(c_idx > 0 && c_idx <= numCameras()) {
					_inst.pipelines[p_idx]->setCamera(_inst.cameras[c_idx]);
				}
			}
			for(size_t i = 0; i < _inst.streams.size(); i++) {
				_inst.streams[i].syncIdx();
			}
			if(VisionServer::ntable()->GetEntry("Enable Processing").GetBoolean(false) &&
				_inst.pipelines[p_idx]->input.GrabFrame(frame)
			) {
				beg_proc = std::chrono::high_resolution_clock::now();
				_inst.pipelines[p_idx]->process(frame);
				end_proc = std::chrono::high_resolution_clock::now();
				int verbosity = VisionServer::ntable()->GetEntry("Statistics Verbosity").GetDouble(0);
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
				_inst.pipelines[p_idx]->PutFrame(frame);
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
		VisionServer::ntable()->GetEntry("Status").SetString("Offline");
		VisionServer::ntable()->Delete("Enable Processing");
		VisionServer::ntable()->Delete("Camera Index");
		VisionServer::ntable()->Delete("Pipeline Index");
		VisionServer::ntable()->Delete("Statistics Verbosity");
		VisionServer::ntable()->Delete("Target FPS");

		return true;
	}
	return false;
}
bool VisionServer::runThread(uint16_t rps) {
	VisionServer& _inst = getInstance();
	if(!VisionServer::isRunning() && !_inst.head.joinable()) {
		_inst.head = std::move(std::thread(VisionServer::run, rps));
		return true;
	}
	return false;
}
bool VisionServer::runSingleThread(uint16_t rps) {
	VisionServer& _inst = getInstance();
	if(!VisionServer::isRunning() && !_inst.head.joinable()) {
		_inst.head = std::move(std::thread(VisionServer::runSingle, rps));
		return true;
	}
	return false;
}
bool VisionServer::stop() {
	VisionServer& _inst = getInstance();
	_inst.is_running = false;
	if(_inst.head.joinable()) {
		_inst.head.join();
		return true;
	}
	return false;
}