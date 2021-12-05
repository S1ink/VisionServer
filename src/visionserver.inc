#include "visionserver.h"

template<class pipeline_t>
void VisionServer::runVision(int8_t quality) {
	static_assert(std::is_base_of<PipelineBase, pipeline_t>::value, "Template argument (pipeline_t) must inherit from PipelineBase");

	this->stream.SetCompression(quality);

	pipeline_t pipeline(this);
	
	this->table->PutBoolean("Show Threshold", false);
	this->table->PutBoolean("Show Statistics", false);
	
	cv::Mat frame(this->getCurrentResolution(), CV_8UC3);
	
	this->start = CHRONO::high_resolution_clock::now();
	while(this->runloop) {
		this->source.GrabFrame(frame);

		this->beg = CHRONO::high_resolution_clock::now();
		pipeline.process(frame, this->table->GetEntry("Show Threshold").GetBoolean(false));
		this->end = CHRONO::high_resolution_clock::now();
		this->total_frames++;

		this->total_time = CHRONO::duration<double>(this->end - this->start).count();
		this->frame_time = CHRONO::duration<double>(this->end - this->beg).count();
		this->loop_time = CHRONO::duration<double>(this->end - this->last).count();
		this->last = this->end;

		this->fps = 1.f/this->loop_time;
		if((int)this->total_time > (int)this->sec1_time) {
			this->fps_1s = ((this->total_frames - this->sec1_frames) / (this->total_time - this->sec1_time));
			this->sec1_time = this->total_time;
			this->sec1_frames = this->total_frames;
		}

		if(this->table->GetEntry("Show Statistics").GetBoolean(false)) {
			this->putStats(frame);
		}
		this->output.PutFrame(frame);
	}
	this->runloop = true;
	
	this->table->Delete("Show Threshold");
	this->table->Delete("Show Statistics");
}
template<class pipeline_t1, class pipeline_t2>
void VisionServer::runVision(int8_t quality) {
	static_assert(std::is_base_of<PipelineBase, pipeline_t1>::value, "Template argument (pipeline_t1) must inherit from PipelineBase");
	static_assert(std::is_base_of<PipelineBase, pipeline_t2>::value, "Template argument (pipeline_t2) must inherit from PipelineBase");
	
	this->stream.SetCompression(quality);

	pipeline_t1 p1(this);
	pipeline_t2 p2(this);
	PipelineBase pbase(this);
	
	this->table->PutNumber("Pipeline Index", 0);
	this->table->PutNumber("Pipelines Available", 2);
	this->table->PutBoolean("Show Threshold", false);
	this->table->PutBoolean("Show Statistics", false);

	cv::Mat frame(this->getCurrentResolution(), CV_8UC3);

	this->start = CHRONO::high_resolution_clock::now();
	while(this->runloop) {
		this->source.GrabFrame(frame);
		switch((int8_t)this->table->GetEntry("Pipeline Index").GetDouble(-1)) {
			case 0 : {
				this->beg = CHRONO::high_resolution_clock::now();
				p1.process(frame, this->table->GetEntry("Show Threshold").GetBoolean(false));
				this->end = CHRONO::high_resolution_clock::now();
				break;
			}
			case 1 : {
				this->beg = CHRONO::high_resolution_clock::now();
				p2.process(frame, this->table->GetEntry("Show Threshold").GetBoolean(false));
				this->end = CHRONO::high_resolution_clock::now();
				break;
			}
			case -1 :
			default : {
				std::cout << "Pipline index out of bounds, please only use values >0 and <Piplines Available\n";
				this->beg = CHRONO::high_resolution_clock::now();
				pbase.process(frame, this->table->GetEntry("Show Threshold").GetBoolean(false));
				this->end = CHRONO::high_resolution_clock::now();
			}
		}
		this->total_frames++;

		this->total_time = CHRONO::duration<double>(this->end - this->start).count();
		this->frame_time = CHRONO::duration<double>(this->end - this->beg).count();
		this->loop_time = CHRONO::duration<double>(this->end - this->last).count();
		this->last = this->end;

		this->fps = 1.f/this->loop_time;
		if((int)this->total_time > (int)this->sec1_time) {
			this->fps_1s = ((this->total_frames - this->sec1_frames) / (this->total_time - this->sec1_time));
			this->sec1_time = this->total_time;
			this->sec1_frames = this->total_frames;
		}

		if(this->table->GetEntry("Show Statistics").GetBoolean(false)) {
			this->putStats(frame);
		}
		this->output.PutFrame(frame);
	}
	this->runloop = true;
	
	this->table->Delete("Pipeline Index");
	this->table->Delete("Piplines Available");
	this->table->Delete("Show Threshold");
	this->table->Delete("Show Statistics");
}
template<class pipeline_t1, class pipeline_t2, class pipeline_t3>
void VisionServer::runVision(int8_t quality) {
	static_assert(std::is_base_of<PipelineBase, pipeline_t1>::value, "Template argument (pipeline_t1) must inherit from PipelineBase");
	static_assert(std::is_base_of<PipelineBase, pipeline_t2>::value, "Template argument (pipeline_t2) must inherit from PipelineBase");
	static_assert(std::is_base_of<PipelineBase, pipeline_t3>::value, "Template argument (pipeline_t3) must inherit from PipelineBase");
	
	this->stream.SetCompression(quality);

	pipeline_t1 p1(this);
	pipeline_t2 p2(this);
	pipeline_t3 p3(this);
	PipelineBase pbase(this);
	
	this->table->PutNumber("Pipeline Index", 0);
	this->table->PutNumber("Pipelines Available", 3);
	this->table->PutBoolean("Show Threshold", false);
	this->table->PutBoolean("Show Statistics", false);

	cv::Mat frame(this->getCurrentResolution(), CV_8UC3);

	this->start = CHRONO::high_resolution_clock::now();
	while(this->runloop) {
		this->source.GrabFrame(frame);
		switch((int8_t)this->table->GetEntry("Pipeline Index").GetDouble(-1)) {
			case 0 : {
				this->beg = CHRONO::high_resolution_clock::now();
				p1.process(frame, this->table->GetEntry("Show Threshold").GetBoolean(false));
				this->end = CHRONO::high_resolution_clock::now();
				break;
			}
			case 1 : {
				this->beg = CHRONO::high_resolution_clock::now();
				p2.process(frame, this->table->GetEntry("Show Threshold").GetBoolean(false));
				this->end = CHRONO::high_resolution_clock::now();
				break;
			}
			case 2 : {
				this->beg = CHRONO::high_resolution_clock::now();
				p3.process(frame, this->table->GetEntry("Show Threshold").GetBoolean(false));
				this->end = CHRONO::high_resolution_clock::now();
			}
			case -1 :
			default : {
				std::cout << "Pipline index out of bounds, please only use values >0 and <'Piplines Available'\n";
				this->beg = CHRONO::high_resolution_clock::now();
				PipelineBase(this).process(frame, this->table->GetEntry("Show Threshold").GetBoolean(false));
				this->end = CHRONO::high_resolution_clock::now();
			}
		}
		this->total_frames++;

		this->total_time = CHRONO::duration<double>(this->end - this->start).count();
		this->frame_time = CHRONO::duration<double>(this->end - this->beg).count();
		this->loop_time = CHRONO::duration<double>(this->end - this->last).count();
		this->last = this->end;

		this->fps = 1.f/this->loop_time;
		if((int)this->total_time > (int)this->sec1_time) {
			this->fps_1s = ((this->total_frames - this->sec1_frames) / (this->total_time - this->sec1_time));
			this->sec1_time = this->total_time;
			this->sec1_frames = this->total_frames;
		}

		if(this->table->GetEntry("Show Statistics").GetBoolean(false)) {
			this->putStats(frame);
		}
		this->output.PutFrame(frame);
	}
	this->runloop = true;
	
	this->table->Delete("Pipeline Index");
	this->table->Delete("Piplines Available");
	this->table->Delete("Show Threshold");
	this->table->Delete("Show Statistics");
}
template<class pipeline_t>
bool VisionServer::runVisionThread(int8_t quality) {
	static_assert(std::is_base_of<PipelineBase, pipeline_t>::value, "Template argument (pipeline_t) must inherit from PipelineBase");
	if(!this->launched.joinable()) {
		this->launched = std::move(std::thread(VisionServer::visionWorker<pipeline_t>, this, quality));
		return true;
	}
	return false;
}
template<typename pipeline_t>
void VisionServer::visionWorker(VisionServer* server, int8_t quality) {

	server->stream.SetCompression(quality);

	pipeline_t pipeline(server);	
	
	server->table->PutBoolean("Show Threshold", false);
	server->table->PutBoolean("Show Statistics", false);
	
	cv::Mat frame(server->getCurrentResolution(), CV_8UC3);
	
	server->start = CHRONO::high_resolution_clock::now();
	while(server->runloop) {
		server->source.GrabFrame(frame);

		server->beg = CHRONO::high_resolution_clock::now();
		pipeline.process(frame, server->table->GetEntry("Show Threshold").GetBoolean(false));
		server->end = CHRONO::high_resolution_clock::now();
		server->total_frames++;

		server->total_time = CHRONO::duration<double>(server->end - server->start).count();
		server->frame_time = CHRONO::duration<double>(server->end - server->beg).count();
		server->loop_time = CHRONO::duration<double>(server->end - server->last).count();
		server->last = server->end;

		server->fps = 1.f/server->loop_time;
		if((int)server->total_time > (int)server->sec1_time) {
			server->fps_1s = ((server->total_frames - server->sec1_frames) / (server->total_time - server->sec1_time));
			server->sec1_time = server->total_time;
			server->sec1_frames = server->total_frames;
		}

		if(server->table->GetEntry("Show Statistics").GetBoolean(false)) {
			server->putStats(frame);
		}
		server->output.PutFrame(frame);
	}
	server->runloop = true;
	
	server->table->Delete("Show Threshold");
	server->table->Delete("Show Statistics");
}