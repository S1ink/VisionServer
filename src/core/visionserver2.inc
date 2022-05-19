#include "visionserver2.h"

#include <type_traits>


//using namespace vs2;
namespace vs2 {

inline const std::vector<VisionCamera>& VisionServer::getCameras() { return inst().cameras; }
inline size_t VisionServer::numCameras() { return inst().cameras.size(); }
inline const std::vector<VisionServer::BasePipe*>& VisionServer::getPipelines() { return inst().pipelines; }
inline size_t VisionServer::numPipelines() { return inst().pipelines.size(); }
inline const std::vector<VisionServer::OutputStream>& VisionServer::getStreams() { return inst().streams; }
inline size_t VisionServer::numStreams() { return inst().streams.size(); }
inline bool VisionServer::isRunning() { return inst().is_running; }

template<class pipeline>
void VisionServer::addPipeline() {
	static_assert(std::is_base_of<VisionServer::BasePipe, pipeline>::value, "Template argument (pipeline) must inherit from BasePipe");
	static_assert(std::is_default_constructible<pipeline>::value, "Template arguement (pipeline) must be default constructible");
	if(!inst().is_running) {
		inst().heap_allocated.emplace_back(std::make_unique<pipeline>());
		inst().pipelines.push_back(inst().heap_allocated.back().get());
	}
}
template<class... pipelines_t>
void VisionServer::addPipelines() {
	if(!inst().is_running) {
		inst().heap_allocated.reserve(inst().heap_allocated.size() + sizeof(pipelines_t...));
		inst().pipelines.reserve(inst().pipelines.size() + sizeof(pipelines_t...));
		size_t alloc = VisionServer::pipeExpander<pipelines_t...>(inst().heap_allocated);
		for(size_t i = alloc; i >= 0; --i) {
			inst().pipelines.push_back(inst().heap_allocated.at(inst().heap_allocated.size() - i - 1).get());
		}
	}
}
template<class... pipelines_t>
void VisionServer::setPipelines() {
	if(!inst().is_running) {
		inst().heap_allocated.clear();
		inst().heap_allocated.reserve(sizeof(pipelines_t...));
		inst().pipelines.clear();
		inst().pipelines.reserve(sizeof(pipelines_t...));
		VisionServer::pipeExpander<pipelines_t...>(inst().heap_allocated);
		for(size_t i = 0; i < inst().heap_allocated.size(); i++) {
			inst().pipelines.push_back(inst().heap_allocated.at(i).get());
		}
	}
}

template<class pipeline, class... pipelines>
size_t VisionServer::pipeExpander(std::vector<std::unique_ptr<BasePipe> >& pipes) {
	if constexpr(!std::is_same<pipeline, void>::value) {
		static_assert(std::is_base_of<VisionServer::BasePipe, pipeline>::value, "Template argument (pipeline) must inherit from BasePipe");
		static_assert(std::is_default_constructible<pipeline>::value, "Template arguement (pipeline) must be default constructible");

		pipes.emplace_back(std::make_unique<pipeline>());
		return 1U + VisionServer::pipeExpander<pipelines...>(pipes);
	}
	return 0U;
}

template<class derived>
void VPipeline<derived>::process(cv::Mat&) {}

template<class... pipelines>
inline std::string SequentialPipeline<pipelines...>::InitPipelines(std::vector<std::unique_ptr<VisionServer::BasePipe> >& pipes) {
	VisionServer::pipeExpander<pipelines...>(pipes);
	std::string fullname = "{ ";
	for(size_t i = 0; i < pipes.size(); i++) {
		fullname += pipes.at(i)->getName() + " ";
	}
	fullname += "}";
	return fullname;
}
template<class... pipelines>
void SequentialPipeline<pipelines...>::process(cv::Mat& io_frame) {
	for(size_t i = 0; i < this->pipelines.size(); i++) {
		this->pipelines.at(i)->process(io_frame);
	}
}

}