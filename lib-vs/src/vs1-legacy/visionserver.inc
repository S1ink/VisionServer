#include "visionserver.h"

#include <type_traits>


namespace vs1 {

inline void VPipeline::updateTarget(const std::string& target) { this->env->updateTarget(target); }
inline const VisionCamera& VPipeline::getCurrentCamera() const { return this->env->getCurrentCamera(); }
inline const cv::Mat_<float>& VPipeline::getCameraMatrix() const { return this->getCurrentCamera().getCameraMatrix(); }
inline const cv::Mat_<float>& VPipeline::getCameraDistortion() const { return this->getCurrentCamera().getDistortionCoefs(); }


template<class pipeline_t>
bool VisionServer::runVisionThread_S(int8_t quality) {
	if(!this->launched.joinable()) {
		this->launched = std::move(std::thread(VisionServer::visionWorker_S<pipeline_t>, std::ref(*this), quality));
		return true;
	}
	return false;
}
template<class pipeline_t1, class pipeline_t2>
bool VisionServer::runVisionThread_S(int8_t quality) {
	if(!this->launched.joinable()) {
		this->launched = std::move(std::thread(VisionServer::visionWorker_S<pipeline_t1, pipeline_t2>, std::ref(*this), quality));
		return true;
	}
	return false;
}
template<class pipeline_t1, class pipeline_t2, class pipeline_t3>
bool VisionServer::runVisionThread_S(int8_t quality) {
	if(!this->launched.joinable()) {
		this->launched = std::move(std::thread(VisionServer::visionWorker_S<pipeline_t1, pipeline_t2, pipeline_t3>, std::ref(*this), quality));
		return true;
	}
	return false;
}
template<class... pipelines>
bool VisionServer::runVisionThread(int8_t quality) {
	if(!this->launched.joinable()) {
		this->launched = std::move(std::thread(VisionServer::visionWorker<pipelines...>, std::ref(*this), quality));
		return true;
	}
	return false;
}

template<class pipeline_t>
void VisionServer::visionWorker_S(VisionServer& server, int8_t quality) {
	static_assert(std::is_base_of<VPipeline, pipeline_t>::value, "Template argument (pipeline_t) must inherit from VPipeline");

	server.stream.SetCompression(quality);
	server.stream.SetDefaultCompression(quality);
	server.vision->PutBoolean("Show Statistics", false);
	server.vision->PutBoolean("Enable Processing", true);
	
	cv::Mat frame(server.getCurrentResolution(), CV_8UC3);
	pipeline_t pipeline(server);

	server.start = CHRONO::high_resolution_clock::now();
	while(server.runloop) {
		server.getCurrentCamera().getFrame(frame);

		server.beg = CHRONO::high_resolution_clock::now();
		if(server.vision->GetBoolean("Enable Processing", true)) {
			pipeline.process(frame);
		}
		server.end = CHRONO::high_resolution_clock::now();
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

		nt::NetworkTableInstance::GetDefault().Flush();
	}
	server.vision->Delete("Show Statistics");
	server.vision->Delete("Enable Processing");
	server.runloop = true;
}
template<class pipeline_t1, class pipeline_t2>
void VisionServer::visionWorker_S(VisionServer& server, int8_t quality) {
	static_assert(std::is_base_of<VPipeline, pipeline_t1>::value, "Template argument (pipeline_t1) must inherit from VPipeline");
	static_assert(std::is_base_of<VPipeline, pipeline_t2>::value, "Template argument (pipeline_t2) must inherit from VPipeline");
	
	server.stream.SetCompression(quality);
	server.stream.SetDefaultCompression(quality);
	server.vision->PutNumber("Pipeline Index", 0);
	server.vision->PutNumber("Pipelines Available", 2);
	server.vision->PutBoolean("Show Statistics", false);
	server.vision->PutBoolean("Enable Processing", true);

	cv::Mat frame(server.getCurrentResolution(), CV_8UC3);
	pipeline_t1 p1(server);
	pipeline_t2 p2(server);

	server.start = CHRONO::high_resolution_clock::now();
	while(server.runloop) {
		server.getCurrentCamera().getFrame(frame);
		switch(
			(server.vision->GetBoolean("Enable Processing", true)) * 
				((int8_t)server.vision->GetEntry("Pipeline Index").GetDouble(-1) + 1)
		) {
			case 1 : {
				server.beg = CHRONO::high_resolution_clock::now();
				p1.process(frame);
				server.end = CHRONO::high_resolution_clock::now();
				break;
			}
			case 2 : {
				server.beg = CHRONO::high_resolution_clock::now();
				p2.process(frame);
				server.end = CHRONO::high_resolution_clock::now();
				break;
			}
			default : {	// includes case 0
				//std::cout << "Pipline index out of bounds, please only use values >0 and <Piplines Available\n";
				server.beg = server.end = CHRONO::high_resolution_clock::now();
			}
		}
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

		nt::NetworkTableInstance::GetDefault().Flush();
	}
	server.vision->Delete("Pipeline Index");
	server.vision->Delete("Piplines Available");
	server.vision->Delete("Show Statistics");
	server.vision->Delete("Enable Processing");
	server.runloop = true;
}
template<class pipeline_t1, class pipeline_t2, class pipeline_t3>
void VisionServer::visionWorker_S(VisionServer& server, int8_t quality) {
	static_assert(std::is_base_of<VPipeline, pipeline_t1>::value, "Template argument (pipeline_t1) must inherit from VPipeline");
	static_assert(std::is_base_of<VPipeline, pipeline_t2>::value, "Template argument (pipeline_t2) must inherit from VPipeline");
	static_assert(std::is_base_of<VPipeline, pipeline_t3>::value, "Template argument (pipeline_t3) must inherit from VPipeline");
	
	server.stream.SetCompression(quality);
	server.stream.SetDefaultCompression(quality);
	server.vision->PutNumber("Pipeline Index", 0);
	server.vision->PutNumber("Pipelines Available", 3);
	server.vision->PutBoolean("Show Statistics", false);
	server.vision->PutBoolean("Enable Processing", true);

	cv::Mat frame(server.getCurrentResolution(), CV_8UC3);
	pipeline_t1 p1(server);
	pipeline_t2 p2(server);
	pipeline_t3 p3(server);

	server.start = CHRONO::high_resolution_clock::now();
	while(server.runloop) {
		server.getCurrentCamera().getFrame(frame);
		switch(
			server.vision->GetBoolean("Enable Processing", true) * 
				((int8_t)server.vision->GetEntry("Pipeline Index").GetDouble(-1) + 1)
		) {
			case 1 : {
				server.beg = CHRONO::high_resolution_clock::now();
				p1.process(frame);
				server.end = CHRONO::high_resolution_clock::now();
				break;
			}
			case 2 : {
				server.beg = CHRONO::high_resolution_clock::now();
				p2.process(frame);
				server.end = CHRONO::high_resolution_clock::now();
				break;
			}
			case 3 : {
				server.beg = CHRONO::high_resolution_clock::now();
				p3.process(frame);
				server.end = CHRONO::high_resolution_clock::now();
				break;
			}
			case 0 :
			default : {
				//std::cout << "Pipline index out of bounds, please only use values >0 and <'Piplines Available'\n";
				server.beg = server.end = CHRONO::high_resolution_clock::now();
			}
		}
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

		nt::NetworkTableInstance::GetDefault().Flush();
	}
	server.vision->Delete("Pipeline Index");
	server.vision->Delete("Piplines Available");
	server.vision->Delete("Show Statistics");
	server.vision->Delete("Enable Processing");
	server.runloop = true;
}

template<class... pipelines>
void VisionServer::visionWorker(VisionServer& server, int8_t quality) {

	std::vector<std::unique_ptr<VPipeline> > pipes;
	pipeExpansion<pipelines...>(pipes, server);

	server.stream.SetCompression(quality);
	server.stream.SetDefaultCompression(quality);
	server.vision->PutNumber("Pipeline Index", 0);
	server.vision->PutNumber("Pipelines Available", pipes.size());
	server.vision->PutBoolean("Show Statistics", false);
	server.vision->PutBoolean("Enable Processing", true);

	cv::Mat frame(server.getCurrentResolution(), CV_8UC3);

	server.start = CHRONO::high_resolution_clock::now();
	while(server.runloop) {
		server.getCurrentCamera().getFrame(frame);

		int idx = server.vision->GetBoolean("Enable Processing", true) * ((int)server.vision->GetEntry("Pipeline Index").GetDouble(-1) + 1);
		if(idx > 0 && idx <= pipes.size()) {
			server.beg = CHRONO::high_resolution_clock::now();
			pipes[idx - 1]->process(frame);
			server.end = CHRONO::high_resolution_clock::now();
		} else {
			server.beg = server.end = CHRONO::high_resolution_clock::now();
		}
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

		nt::NetworkTableInstance::GetDefault().Flush();
	}
	server.vision->Delete("Pipeline Index");
	server.vision->Delete("Piplines Available");
	server.vision->Delete("Show Statistics");
	server.vision->Delete("Enable Processing");
	server.runloop = true;

}

template<class pipeline, class... pipelines>
void VisionServer::pipeExpansion(std::vector<std::unique_ptr<VPipeline> >& pipes, VisionServer& server) {
	if constexpr(!std::is_same<pipeline, void>::value) {
		static_assert(std::is_base_of<VPipeline, pipeline>::value, "Template argument (pipeline) must inherit from VPipeline");

		pipes.emplace_back(std::make_unique<pipeline>(server));
		pipeExpansion<pipelines...>(pipes, server);
	}
}

}	// namespace vs1