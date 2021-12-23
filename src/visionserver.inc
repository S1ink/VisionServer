#include "visionserver.h"

#include <type_traits>

//#include "pipelines.h"

template<class pipeline_t>
void VisionServer::runVision(int8_t quality) {
	VisionServer::visionWorker<pipeline_t>(*this, quality);
}
template<class pipeline_t1, class pipeline_t2>
void VisionServer::runVision(int8_t quality) {
	VisionServer::visionWorker<pipeline_t1, pipeline_t2>(*this, quality);
}
template<class pipeline_t1, class pipeline_t2, class pipeline_t3>
void VisionServer::runVision(int8_t quality) {
	VisionServer::visionWorker<pipeline_t1, pipeline_t2, pipeline_t3>(*this, quality);
}

template<class pipeline_t>
bool VisionServer::runVisionThread(int8_t quality) {
	if(!this->launched.joinable()) {
		this->launched = std::move(std::thread(VisionServer::visionWorker<pipeline_t>, *this, quality));
		return true;
	}
	return false;
}
template<class pipeline_t1, class pipeline_t2>
bool VisionServer::runVisionThread(int8_t quality) {
	if(!this->launched.joinable()) {
		this->launched = std::move(std::thread(VisionServer::visionWorker<pipeline_t1, pipeline_t2>, *this, quality));
		return true;
	}
	return false;
}
template<class pipeline_t1, class pipeline_t2, class pipeline_t3>
bool VisionServer::runVisionThread(int8_t quality) {
	if(!this->launched.joinable()) {
		this->launched = std::move(std::thread(VisionServer::visionWorker<pipeline_t1, pipeline_t2, pipeline_t3>, *this, quality));
		return true;
	}
	return false;
}

template<class pipeline_t>
void VisionServer::visionWorker(VisionServer& server, int8_t quality) {
	static_assert(std::is_base_of<VPipeline, pipeline_t>::value, "Template argument (pipeline_t) must implement VPipeline");

	server.stream.SetCompression(quality);

	pipeline_t pipeline(server);
	
	server.table->PutBoolean("Show Statistics", false);
	server.setupNtVflag();
	
	cv::Mat frame(server.getCurrentResolution(), CV_8UC3);
	
	server.start = CHRONO::high_resolution_clock::now();
	while(server.runloop) {
		server.source.GrabFrame(frame);

		server.beg = CHRONO::high_resolution_clock::now();
		pipeline.process(frame, server.getNtVflag());
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

		if(server.table->GetEntry("Show Statistics").GetBoolean(false)) {
			server.putStats(frame);
		}
		server.updateStats();
		server.output.PutFrame(frame);
	}
	server.runloop = true;
	
	server.table->Delete("Show Statistics");
	server.deleteNtVflag();
}
template<class pipeline_t1, class pipeline_t2>
void VisionServer::visionWorker(VisionServer& server, int8_t quality) {
	static_assert(std::is_base_of<VPipeline, pipeline_t1>::value, "Template argument (pipeline_t1) must implement VPipeline");
	static_assert(std::is_base_of<VPipeline, pipeline_t2>::value, "Template argument (pipeline_t2) must implement VPipeline");
	
	server.stream.SetCompression(quality);

	pipeline_t1 p1(server);
	pipeline_t2 p2(server);
	DefaultPipeline pbase(server);
	
	server.table->PutNumber("Pipeline Index", 0);
	server.table->PutNumber("Pipelines Available", 2);
	server.table->PutBoolean("Show Statistics", false);
	server.setupNtVflag();

	cv::Mat frame(server.getCurrentResolution(), CV_8UC3);

	server.start = CHRONO::high_resolution_clock::now();
	while(server.runloop) {
		server.source.GrabFrame(frame);
		switch((int8_t)server.table->GetEntry("Pipeline Index").GetDouble(-1)) {
			case 0 : {
				server.beg = CHRONO::high_resolution_clock::now();
				p1.process(frame, server.getNtVflag());
				server.end = CHRONO::high_resolution_clock::now();
				break;
			}
			case 1 : {
				server.beg = CHRONO::high_resolution_clock::now();
				p2.process(frame, server.getNtVflag());
				server.end = CHRONO::high_resolution_clock::now();
				break;
			}
			case -1 :
			default : {
				//std::cout << "Pipline index out of bounds, please only use values >0 and <Piplines Available\n";
				server.beg = CHRONO::high_resolution_clock::now();
				pbase.process(frame, server.getNtVflag());
				server.end = CHRONO::high_resolution_clock::now();
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

		if(server.table->GetEntry("Show Statistics").GetBoolean(false)) {
			server.putStats(frame);
		}
		server.updateStats();
		server.output.PutFrame(frame);
	}
	server.runloop = true;
	
	server.table->Delete("Pipeline Index");
	server.table->Delete("Piplines Available");
	server.table->Delete("Show Statistics");
	server.deleteNtVflag();
}
template<class pipeline_t1, class pipeline_t2, class pipeline_t3>
void VisionServer::visionWorker(VisionServer& server, int8_t quality) {

	static_assert(std::is_base_of<VPipeline, pipeline_t1>::value, "Template argument (pipeline_t1) must implement VPipeline");
	static_assert(std::is_base_of<VPipeline, pipeline_t2>::value, "Template argument (pipeline_t2) must implement VPipeline");
	static_assert(std::is_base_of<VPipeline, pipeline_t3>::value, "Template argument (pipeline_t3) must implement VPipeline");
	
	server.stream.SetCompression(quality);

	pipeline_t1 p1(server);
	pipeline_t2 p2(server);
	pipeline_t3 p3(server);
	DefaultPipeline pbase(server);
	
	server.table->PutNumber("Pipeline Index", 0);
	server.table->PutNumber("Pipelines Available", 3);
	server.table->PutBoolean("Show Statistics", false);
	server.setupNtVflag();

	cv::Mat frame(server.getCurrentResolution(), CV_8UC3);

	server.start = CHRONO::high_resolution_clock::now();
	while(server.runloop) {
		server.source.GrabFrame(frame);
		switch((int8_t)server.table->GetEntry("Pipeline Index").GetDouble(-1)) {
			case 0 : {
				server.beg = CHRONO::high_resolution_clock::now();
				p1.process(frame, server.getNtVflag());
				server.end = CHRONO::high_resolution_clock::now();
				break;
			}
			case 1 : {
				server.beg = CHRONO::high_resolution_clock::now();
				p2.process(frame, server.getNtVflag());
				server.end = CHRONO::high_resolution_clock::now();
				break;
			}
			case 2 : {
				server.beg = CHRONO::high_resolution_clock::now();
				p3.process(frame, server.getNtVflag());
				server.end = CHRONO::high_resolution_clock::now();
				break;
			}
			case -1 :
			default : {
				//std::cout << "Pipline index out of bounds, please only use values >0 and <'Piplines Available'\n";
				server.beg = CHRONO::high_resolution_clock::now();
				pbase.process(frame, server.getNtVflag());
				server.end = CHRONO::high_resolution_clock::now();
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

		if(server.table->GetEntry("Show Statistics").GetBoolean(false)) {
			server.putStats(frame);
		}
		server.updateStats();
		server.output.PutFrame(frame);
	}
	server.runloop = true;
	
	server.table->Delete("Pipeline Index");
	server.table->Delete("Piplines Available");
	server.table->Delete("Show Statistics");
	server.deleteNtVflag();
}