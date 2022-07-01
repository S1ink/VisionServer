#include "visionserver2.h"

#include <type_traits>
#include <sstream>


namespace vs2 {

inline const std::vector<VisionCamera>& VisionServer::getCameras() { return inst().cameras; }
inline size_t VisionServer::numCameras() { return inst().cameras.size(); }
inline const std::vector<VisionServer::BasePipe*>& VisionServer::getPipelines() { return inst().pipelines; }
inline size_t VisionServer::numPipelines() { return inst().pipelines.size(); }
inline const std::vector<VisionServer::OutputStream>& VisionServer::getStreams() { return inst().streams; }
inline size_t VisionServer::numStreams() { return inst().streams.size(); }
inline bool VisionServer::isRunning() { return inst().is_running; }

template<class pipeline>
bool VisionServer::addPipeline() {
	static_assert(std::is_base_of<VisionServer::BasePipe, pipeline>::value, "Template argument (pipeline) must inherit from BasePipe");
	static_assert(std::is_default_constructible<pipeline>::value, "Template arguement (pipeline) must be default constructible");
	if(!VisionServer::isRunning()) {
		VisionServer& _inst = getInstance();
		_inst.heap_allocated.emplace_back(std::make_unique<pipeline>());
		_inst.pipelines.push_back(_inst.heap_allocated.back().get());
		return true;
	}
	return false;
}
template<class... pipelines_t>
bool VisionServer::addPipelines() {
	if(!VisionServer::isRunning()) {
		VisionServer& _inst = getInstance();
		_inst.heap_allocated.reserve(_inst.heap_allocated.size() + sizeof...(pipelines_t));
		_inst.pipelines.reserve(_inst.pipelines.size() + sizeof...(pipelines_t));
		size_t alloc = VisionServer::pipeExpander<pipelines_t...>(inst().heap_allocated);
		for(size_t i = alloc; i >= 0; --i) {
			_inst.pipelines.push_back(_inst.heap_allocated.at(_inst.heap_allocated.size() - i - 1).get());
		}
		return true;
	}
	return false;
}
template<class... pipelines_t>
bool VisionServer::setPipelines() {
	if(!VisionServer::isRunning()) {
		VisionServer& _inst = getInstance();
		_inst.heap_allocated.clear();
		_inst.heap_allocated.reserve(sizeof...(pipelines_t));
		_inst.pipelines.clear();
		_inst.pipelines.reserve(sizeof...(pipelines_t));
		VisionServer::pipeExpander<pipelines_t...>(_inst.heap_allocated);
		for(size_t i = 0; i < _inst.heap_allocated.size(); i++) {
			_inst.pipelines.push_back(_inst.heap_allocated.at(i).get());
		}
		return true;
	}
	return false;
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

template<class... pipelines_t>
inline std::string SequentialPipeline<pipelines_t...>::Construct(std::vector<std::unique_ptr<VisionServer::BasePipe> >& pipes) {
	if(VisionServer::pipeExpander<pipelines_t...>(pipes) < 1) {
		return "void";
	} else {
		std::stringstream ret(pipes.at(0)->getName());
		for(size_t i = 1; i < pipes.size(); i++) {
			ret << ", " << pipes.at(i)->getName();
		}
		return ret.str();
	}
	
}
template<class... pipelines_t>
void SequentialPipeline<pipelines_t...>::addPipeline(VisionServer::BasePipe* p) {
	this->pipelines.push_back(p);
}
template<class... pipelines_t>
void SequentialPipeline<pipelines_t...>::addPipelines(std::vector<VisionServer::BasePipe*>&& ps) {
	this->pipelines.insert(this->pipelines.end(), ps.begin(), ps.end());
}
template<class... pipelines_t>
void SequentialPipeline<pipelines_t...>::addPipelines(std::initializer_list<VisionServer::BasePipe*> ps) {
	this->pipelines.insert(this->pipelines.end(), ps.begin(), ps.end());
}
template<class... pipelines_t>
void SequentialPipeline<pipelines_t...>::process(cv::Mat& io_frame) {
	for(size_t i = 0; i < this->pipelines.size(); i++) {
		this->pipelines.at(i)->process(io_frame);
	}
}

}	// namespace vs2;