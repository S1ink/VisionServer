#include "visionserver2.h"

#include <type_traits>


inline const std::vector<VisionCamera>& VisionServer2::getCameras() { return inst().cameras; }
inline size_t VisionServer2::numCameras() { return inst().cameras.size(); }
inline const std::vector<std::unique_ptr<VisionServer2::BasePipe> >& VisionServer2::getPipelines() { return inst().pipelines; }
inline size_t VisionServer2::numPipelines() { return inst().pipelines.size(); }
//inline const std::vector<VisionServer2::OutputStream>& VisionServer2::getStreams() { return inst().streams; }
inline size_t VisionServer2::numStreams() { return inst().streams.size(); }
inline bool VisionServer2::isRunning() { return inst().is_running; }

template<class pipeline>
void VisionServer2::addPipeline() {
	static_assert(std::is_base_of<VisionServer2::BasePipe, pipeline>::value, "Template argument (pipeline) must inherit from BasePipe");
	static_assert(std::is_default_constructible<pipeline>::value, "Template arguement (pipeline) must be default constructible");
	if(!inst().is_running) {
		inst().pipelines.emplace_back(std::make_unique<pipeline>());
	}
}
template<class... pipelines_t>
void VisionServer2::addPipelines() {
	if(!inst().is_running) {
		VisionServer2::pipeExpander<pipelines_t...>(inst().pipelines);
	}
}
template<class... pipelines_t>
void VisionServer2::setPipelines() {
	if(!inst().is_running) {
		inst().pipelines.clear();
		VisionServer2::pipeExpander<pipelines_t...>(inst().pipelines);
	}
}

template<class pipeline, class... pipelines>
void VisionServer2::pipeExpander(std::vector<std::unique_ptr<BasePipe> >& pipes) {
	if constexpr(!std::is_same<pipeline, void>::value) {
		static_assert(std::is_base_of<VisionServer2::BasePipe, pipeline>::value, "Template argument (pipeline) must inherit from BasePipe");
		static_assert(std::is_default_constructible<pipeline>::value, "Template arguement (pipeline) must be default constructible");

		pipes.emplace_back(std::make_unique<pipeline>());
		VisionServer2::pipeExpander<pipelines...>(pipes);
	}
}

template<class derived>
void VPipeline2<derived>::process(cv::Mat&) {}