#pragma once

#include <vector>
#include <string>
#include <thread>
#include <fstream>
#include <initializer_list>

#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/interpreter_builder.h>
#include <tensorflow/lite/model_builder.h>
#include <tensorflow/lite/kernels/register.h>
#include <edgetpu.h>
#include <opencv2/opencv.hpp>

#include "tools/src/resources.h"
#include "visionserver2.h"


void loadObjectLabels(const std::string& file, std::vector<std::string>& objs);
inline size_t tpusAvailable() { return edgetpu::EdgeTpuManager::GetSingleton()->EnumerateEdgeTpu().size(); }

/** Provides a base for all TensorFlow[Lite] based processing pipelines */
class TfModel {
public:
	inline static size_t
		default_threading{std::thread::hardware_concurrency() / 2};
	enum class Optimization {
		DEFAULT,
		EDGETPU
	};

	TfModel() = delete;
	TfModel(std::initializer_list<std::pair<const char*, Optimization> >, size_t = default_threading);
	inline TfModel(const char* def, const char* edge, size_t th = default_threading) :
		TfModel({{def, Optimization::DEFAULT}, {edge, Optimization::EDGETPU}}, th) {}
	inline TfModel(const char* m, size_t t = default_threading) :
		TfModel(m, nullptr, t) {}
	TfModel(const TfModel&) = delete;

	inline operator bool() const { return this->isValid(); }
	inline bool isValid() const { return this->map && this->model; }
	inline bool isValidBuffer() const { return !this->input_tensor.empty(); }


protected:
	std::unique_ptr<tflite::FlatBufferModel> map;
	std::unique_ptr<tflite::Interpreter> model;
	std::shared_ptr<edgetpu::EdgeTpuContext> edgetpu_context;
	tflite::ops::builtin::BuiltinOpResolver resolver;

	cv::Mat input_tensor;
	cv::Size input_size;


};



struct AxonRunner_B {
	enum class OutputTensor {
		BOX_COORDS = 0,
		LABEL_IDX = 1,
		CONFIDENCE = 2,
		DETECTIONS = 3
	};

	inline static const char
		*default_model = "unoptimized.tflite",
		*edgetpu_model = "model.tflite",
		*default_labels = "map.pbtxt"
	;
};

/** Runs an Axon-generated or similar object classification tflite model within a vision pipeline */
template<class derived_t = void>
class AxonRunner : public vs2::VPipeline<AxonRunner<derived_t> >, public TfModel, public AxonRunner_B {
	typedef struct AxonRunner<derived_t>	This_t;
public:
	inline AxonRunner(size_t th = default_threading) :
		AxonRunner({{default_model, Optimization::DEFAULT}, {edgetpu_model, Optimization::EDGETPU}}, default_labels, th) {}
	inline AxonRunner(const char* model, Optimization opt = Optimization::DEFAULT, const char* map = default_labels, size_t th = default_threading) :
		AxonRunner({{model, opt}}, map, th) {}
	AxonRunner(std::initializer_list<std::pair<const char*, Optimization> >, const char* map = default_labels, size_t = default_threading);

	inline bool isValidOutput() const { return this->coords && this->labels && this->confidence && this->detections; }

	inline size_t getDetections() const { return this->detections ? *this->detections : -1; }
	inline bool isValidIdx(size_t idx) const { return idx >= 0 && idx < this->getDetections(); }

	virtual void process(cv::Mat& io_frame) override;

protected:
	inline float getX1(size_t idx) const { return this->coords[idx * 4 + 1]; }
	inline float getY1(size_t idx) const { return this->coords[idx * 4]; }
	inline float getX2(size_t idx) const { return this->coords[idx * 4 + 3]; }
	inline float getY2(size_t idx) const { return this->coords[idx * 4 + 2]; }
	inline const std::string& getLabel(size_t idx) const { return this->obj_labels.at(this->labels[idx]); }
	inline uint32_t getConfidence(size_t idx) const { return (this->confidence[idx] * 100.f); }

	std::vector<std::string> obj_labels;
	const float
		*coords,
		*labels,
		*confidence,
		*detections
	;


};



struct MoveNet_B {
	enum class Output {
		NOSE,
		L_EYE, R_EYE,
		L_EAR, R_EAR,
		L_SHOULDER, R_SHOULDER,
		L_ELBOW, R_ELBOW,
		L_WRIST, R_WRIST,
		L_HIP, R_HIP,
		L_KNEE, R_KNEE,
		L_ANKLE, R_ANKLE,
		TOTAL
	};

	inline static const float
		confid_thresh = 0.3;
	inline static const char*
		default_model = "lite-model_movenet_singlepose_lightning_tflite_int8_4.tflite";
	inline static const std::array<const char*, (size_t)Output::TOTAL>
		names{
			"nose",
			"left eye", "right eye",
			"left ear", "right ear",
			"left shoulder", "right shoulder",
			"left elbow", "right elbow",
			"left wrist", "right wrist",
			"left hip", "right hip",
			"left knee", "right knee",
			"left ankle", "right ankle"
		};
	inline static const std::array<std::pair<Output, Output>, 12>
		connections{
			{
				{Output::L_SHOULDER, Output::R_SHOULDER},
				{Output::L_SHOULDER, Output::L_ELBOW},
				{Output::L_ELBOW, Output::L_WRIST},
				{Output::R_SHOULDER, Output::R_ELBOW},
				{Output::R_ELBOW, Output::R_WRIST},
				{Output::L_HIP, Output::R_HIP},
				{Output::L_SHOULDER, Output::L_HIP},
				{Output::R_SHOULDER, Output::R_HIP},
				{Output::L_HIP, Output::L_KNEE},
				{Output::R_HIP, Output::R_KNEE},
				{Output::L_ANKLE, Output::L_KNEE},
				{Output::R_ANKLE, Output::R_KNEE}
			}
		};
};
constexpr inline size_t operator~(MoveNet_B::Output e) { return static_cast<size_t>(e); }

/** Runs a MoveNet pose estimation tflite model within a vision pipeline */
template<class derived_t = void>
class MoveNet : public vs2::VPipeline<MoveNet<derived_t> >, public TfModel, public MoveNet_B {
	typedef struct MoveNet<derived_t>		This_t;
public:
	inline MoveNet(size_t th = default_threading) :
		MoveNet({{default_model, Optimization::DEFAULT}}, th) {}
	inline MoveNet(const char* model, Optimization opt = Optimization::DEFAULT, size_t th = default_threading) :
		MoveNet({{model, opt}}, th) {}
	MoveNet(std::initializer_list<std::pair<const char*, Optimization> >, size_t th = default_threading);

	inline bool isValidOutput() const { return this->outputs; }

	virtual void process(cv::Mat& io_frame) override;

protected:
	inline static bool isValidIdx(size_t idx) { return idx >= 0 && idx < ~Output::TOTAL; }
	inline float getX(size_t idx) const { return this->outputs[idx * 3 + 1]; }
	inline float getY(size_t idx) const { return this->outputs[idx * 3]; }
	inline float getConfidence(size_t idx) const { return this->outputs[idx * 3 + 2]; }

	const float* outputs;
	std::array<cv::Point2f, ~Output::TOTAL> points;


};

#include "tfmodel.inc"