#include "visionserver.h"

template<class pipeline_t>
void VisionServer::runVision() {
	static_assert(std::is_base_of<PipelineBase, pipeline_t>::value, "Template argument (pipeline_t) must inherit from PipelineBase");

	pipeline_t pipeline(this);
	
	this->table->PutBoolean("Show Threshold", false);
	
	cv::Mat frame(this->getCurrentResolution(), CV_8UC3);
	
	this->start = CHRONO::high_resolution_clock::now();
	while(this->runloop) {
		this->source.GrabFrame(frame);
		pipeline.process(frame, this->stream, this->table->GetEntry("Show Threshold").GetBoolean(false));
	}
	this->runloop = true;
	
	this->table->Delete("Show Threshold");
}
template<class pipeline_t1, class pipeline_t2>
void VisionServer::runVision() {
	static_assert(std::is_base_of<PipelineBase, pipeline_t1>::value, "Template argument (pipeline_t1) must inherit from PipelineBase");
	static_assert(std::is_base_of<PipelineBase, pipeline_t2>::value, "Template argument (pipeline_t2) must inherit from PipelineBase");
	
	pipeline_t1 p1(this);
	pipeline_t2 p2(this);
	
	this->table->PutNumber("Pipeline Index", 0);
	this->table->PutNumber("Pipelines Available", 2);
	this->table->PutBoolean("Show Threshold", false);

	cv::Mat frame(this->getCurrentResolution(), CV_8UC3);

	this->start = CHRONO::high_resolution_clock::now();
	while(this->runloop) {
		this->source.GrabFrame(frame);
		switch((int8_t)this->table->GetEntry("Pipeline Index").GetDouble(-1)) {
			case 0 : {
				p1.process(frame, this->stream, this->table->GetEntry("Show Threshold").GetBoolean(false));
				break;
			}
			case 1 : {
				p2.process(frame, this->stream, this->table->GetEntry("Show Threshold").GetBoolean(false));
				break;
			}
			case -1 : {
				std::cout << "Pipline index out of bounds, please only use values >0 and <Piplines Available\n";
			}
			default : {
				PipelineBase(this).process(frame, this->stream, this->table->GetEntry("Show Threshold").GetBoolean(false));
			}
		}
	}
	this->runloop = true;
	
	this->table->Delete("Pipeline Index");
	this->table->Delete("Piplines Available");
	this->table->Delete("Show Threshold");
}
template<class pipeline_t1, class pipeline_t2, class pipeline_t3>
void VisionServer::runVision() {
	static_assert(std::is_base_of<PipelineBase, pipeline_t1>::value, "Template argument (pipeline_t1) must inherit from PipelineBase");
	static_assert(std::is_base_of<PipelineBase, pipeline_t2>::value, "Template argument (pipeline_t2) must inherit from PipelineBase");
	static_assert(std::is_base_of<PipelineBase, pipeline_t3>::value, "Template argument (pipeline_t3) must inherit from PipelineBase");
	
	pipeline_t1 p1(this);
	pipeline_t2 p2(this);
	pipeline_t3 p3(this);
	
	this->table->PutNumber("Pipeline Index", 0);
	this->table->PutNumber("Pipelines Available", 3);
	this->table->PutBoolean("Show Threshold", false);

	cv::Mat frame(this->getCurrentResolution(), CV_8UC3);

	this->start = CHRONO::high_resolution_clock::now();
	while(this->runloop) {
		this->source.GrabFrame(frame);
		switch((int8_t)this->table->GetEntry("Pipeline Index").GetDouble(-1)) {
			case 0 : {
				p1.process(frame, this->stream, this->table->GetEntry("Show Threshold").GetBoolean(false));
				break;
			}
			case 1 : {
				p2.process(frame, this->stream, this->table->GetEntry("Show Threshold").GetBoolean(false));
				break;
			}
			case 2 : {
				p3.process(frame, this->stream, this->table->GetEntry("Show Threshold").GetBoolean(false));
			}
			case -1 : {
				std::cout << "Pipline index out of bounds, please only use values >0 and <'Piplines Available'\n";
			}
			default : {
				PipelineBase(this).process(frame, this->stream, this->table->GetEntry("Show Threshold").GetBoolean(false));
			}
		}
	}
	this->runloop = true;
	
	this->table->Delete("Pipeline Index");
	this->table->Delete("Piplines Available");
	this->table->Delete("Show Threshold");
}
template<class pipeline_t>
bool VisionServer::runVisionThread() {
	static_assert(std::is_base_of<PipelineBase, pipeline_t>::value, "Template argument (pipeline_t) must inherit from PipelineBase");
	if(!this->launched.joinable()) {
		this->launched = std::move(std::thread(VisionServer::visionWorker<pipeline_t>, this));
		return true;
	}
	return false;
}
template<typename pipeline_t>
void VisionServer::visionWorker(VisionServer* server) {

	pipeline_t pipeline(server);

	server->table->PutBoolean("Show Threshold", false);
	
	cv::Mat frame(server->getCurrentResolution(), CV_8UC3);
	
	server->start = CHRONO::high_resolution_clock::now();
	while(server->runloop) {
		server->source.GrabFrame(frame);
		pipeline.process(frame, server->stream, server->table->GetEntry("Show Threshold").GetBoolean(false));
	}
	server->runloop = true;

	server->table->Delete("Show Threshold");
}