#include "visionserver2.h"

#include <type_traits>


//using namespace vs2;
namespace vs2 {

inline const std::vector<VisionCamera>& VisionServer::getCameras() { return inst().cameras; }
inline size_t VisionServer::numCameras() { return inst().cameras.size(); }
inline const std::vector<std::unique_ptr<VisionServer::BasePipe> >& VisionServer::getPipelines() { return inst().pipelines; }
inline size_t VisionServer::numPipelines() { return inst().pipelines.size(); }
inline const std::vector<VisionServer::OutputStream>& VisionServer::getStreams() { return inst().streams; }
inline size_t VisionServer::numStreams() { return inst().streams.size(); }
inline bool VisionServer::isRunning() { return inst().is_running; }

template<class pipeline>
void VisionServer::addPipeline() {
	static_assert(std::is_base_of<VisionServer::BasePipe, pipeline>::value, "Template argument (pipeline) must inherit from BasePipe");
	static_assert(std::is_default_constructible<pipeline>::value, "Template arguement (pipeline) must be default constructible");
	if(!inst().is_running) {
		inst().pipelines.emplace_back(std::make_unique<pipeline>());
	}
}
template<class... pipelines_t>
void VisionServer::addPipelines() {
	if(!inst().is_running) {
		VisionServer::pipeExpander<pipelines_t...>(inst().pipelines);
	}
}
template<class... pipelines_t>
void VisionServer::setPipelines() {
	if(!inst().is_running) {
		inst().pipelines.clear();
		VisionServer::pipeExpander<pipelines_t...>(inst().pipelines);
	}
}

template<class pipeline, class... pipelines>
void VisionServer::pipeExpander(std::vector<std::unique_ptr<BasePipe> >& pipes) {
	if constexpr(!std::is_same<pipeline, void>::value) {
		static_assert(std::is_base_of<VisionServer::BasePipe, pipeline>::value, "Template argument (pipeline) must inherit from BasePipe");
		static_assert(std::is_default_constructible<pipeline>::value, "Template arguement (pipeline) must be default constructible");

		pipes.emplace_back(std::make_unique<pipeline>());
		VisionServer::pipeExpander<pipelines...>(pipes);
	}
}

template<class derived>
void VPipeline<derived>::process(cv::Mat&) {}

}