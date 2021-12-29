// #pragma once

// #include <type_traits>
// #include <tuple>

// #include "visionserver.h"

// template<class... pipelines>
// class PipelinePack {
// protected:
// 	template<typename pipeline_t, typename... piplines>
// 	std::tuple<pipeline_t, pipelines...> initialize(VisionServer& server) {
// 		static_assert(std::is_base_of<VPipeline, pipeline_t>::value, "Pipeline must extend VPipelines");
// 		return std::tuple_cat(std::tuple<pipeline_t>(server), initialize<pipelines...>(server));
// 	}

// public:
// 	PipelinePack(VisionServer& server) : pipes(initialize<pipelines...>(server)) {}

// 	template<size_t I>
// 	auto& getPipeline(void) {
// 		return std::get<I>(this->pipes);
// 	}
// 	template<size_t I>
// 	static auto& getPipeline(PipelinePack& pack) {
// 		return std::get<I>(pack.pipes);
// 	}

// 	size_t size() {
// 		return sizeof...(pipelines);
// 	}

// private:
// 	std::tuple<pipelines...> pipes;

// };