#include "visionserver.h"

#include <type_traits>

// make sure to recopy petra to src/ if we retry implementing this
//#include <petra/include/petra/sequential_table.hpp>
//#include "expirimental.h"
//#include "pipelines.h"

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
	server.stream.SetDefaultCompression(quality);
	server.vision->PutBoolean("Show Statistics", false);
	server.vision->PutBoolean("Enable Processing", true);
	
	cv::Mat frame(server.getCurrentResolution(), CV_8UC3);
	pipeline_t pipeline(server);

	server.start = CHRONO::high_resolution_clock::now();
	while(server.runloop) {
		server.source.GrabFrame(frame);

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
	}
	server.vision->Delete("Show Statistics");
	server.vision->Delete("Enable Processing");
	server.runloop = true;
}
template<class pipeline_t1, class pipeline_t2>
void VisionServer::visionWorker(VisionServer& server, int8_t quality) {
	static_assert(std::is_base_of<VPipeline, pipeline_t1>::value, "Template argument (pipeline_t1) must implement VPipeline");
	static_assert(std::is_base_of<VPipeline, pipeline_t2>::value, "Template argument (pipeline_t2) must implement VPipeline");
	
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
		server.source.GrabFrame(frame);
		switch(server.vision->GetBoolean("Enable Processing", true) * ((int8_t)server.vision->GetEntry("Pipeline Index").GetDouble(-1) + 1)) {
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
	}
	server.vision->Delete("Pipeline Index");
	server.vision->Delete("Piplines Available");
	server.vision->Delete("Show Statistics");
	server.vision->Delete("Enable Processing");
	server.runloop = true;
}
template<class pipeline_t1, class pipeline_t2, class pipeline_t3>
void VisionServer::visionWorker(VisionServer& server, int8_t quality) {
	static_assert(std::is_base_of<VPipeline, pipeline_t1>::value, "Template argument (pipeline_t1) must implement VPipeline");
	static_assert(std::is_base_of<VPipeline, pipeline_t2>::value, "Template argument (pipeline_t2) must implement VPipeline");
	static_assert(std::is_base_of<VPipeline, pipeline_t3>::value, "Template argument (pipeline_t3) must implement VPipeline");
	
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
		server.source.GrabFrame(frame);
		switch(server.vision->GetBoolean("Enable Processing", true) * ((int8_t)server.vision->GetEntry("Pipeline Index").GetDouble(-1) + 1)) {
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
	}
	server.vision->Delete("Pipeline Index");
	server.vision->Delete("Piplines Available");
	server.vision->Delete("Show Statistics");
	server.vision->Delete("Enable Processing");
	server.runloop = true;
}

// NOPE, TOO CONFUSING (trust me I really tried)
// template<typename... pipelines>
// void VisionServer::visionWorker(VisionServer& server, int8_t quality) {
// 	PipelinePack<pipelines...> pack(server);
// 	constexpr auto& pipeAt = petra::make_sequential_table<sizeof...(pipelines)>([&pack](auto&& x){
// 		return PipelinePack<pipelines...>::getPipeline<x()>(pack);
// 	});

// 	server.stream.SetCompression(quality);

// 	//DefaultPipeline pbase(server);
	
// 	server.table->PutNumber("Pipeline Index", 0);
// 	server.table->PutNumber("Pipelines Available", 3);
// 	server.table->PutBoolean("Show Statistics", false);
// 	server.setupNtVflag();

// 	cv::Mat frame(server.getCurrentResolution(), CV_8UC3);

// 	server.start = CHRONO::high_resolution_clock::now();
// 	while(server.runloop) {
// 		server.source.GrabFrame(frame);
		
// 		pipeAt(server.table->GetEntry("Pipeline Index").GetDouble(0)).process(frame, server.getNtVflag());
// 		server.total_frames++;

// 		server.total_time = CHRONO::duration<double>(server.end - server.start).count();
// 		server.frame_time = CHRONO::duration<double>(server.end - server.beg).count();
// 		server.loop_time = CHRONO::duration<double>(server.end - server.last).count();
// 		server.last = server.end;

// 		server.active_target.update(server.loop_time);

// 		server.fps = 1.f/server.loop_time;
// 		if((int)server.total_time > (int)server.sec1_time) {
// 			server.fps_1s = ((server.total_frames - server.sec1_frames) / (server.total_time - server.sec1_time));
// 			server.sec1_time = server.total_time;
// 			server.sec1_frames = server.total_frames;
// 		}

// 		if(server.table->GetEntry("Show Statistics").GetBoolean(false)) {
// 			server.putStats(frame);
// 		}
// 		server.updateStats();
// 		server.output.PutFrame(frame);
// 	}
// 	server.runloop = true;
	
// 	server.table->Delete("Pipeline Index");
// 	server.table->Delete("Piplines Available");
// 	server.table->Delete("Show Statistics");
// 	server.deleteNtVflag();
// }