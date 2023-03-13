#pragma once

#ifndef EXCLUDE_TFLITE

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

#include "cpp-tools/src/resources.h"
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
class AxonRunner_ : public vs2::VPipeline<AxonRunner_<derived_t> >, public TfModel, public AxonRunner_B {
	typedef struct AxonRunner_<derived_t>	This_t;
public:
	inline AxonRunner_(size_t th = default_threading) :
		AxonRunner_({{default_model, Optimization::DEFAULT}, {edgetpu_model, Optimization::EDGETPU}}, default_labels, th) {}
	inline AxonRunner_(const char* model, Optimization opt = Optimization::DEFAULT, const char* map = default_labels, size_t th = default_threading) :
		AxonRunner_({{model, opt}}, map, th) {}
	AxonRunner_(std::initializer_list<std::pair<const char*, Optimization> >, const char* map = default_labels, size_t = default_threading);

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
typedef AxonRunner_<>	AxonRunner;



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
class MoveNet_ : public vs2::VPipeline<MoveNet_<derived_t> >, public TfModel, public MoveNet_B {
	typedef struct MoveNet_<derived_t>		This_t;
public:
	inline MoveNet_(size_t th = default_threading) :
		MoveNet_({{default_model, Optimization::DEFAULT}}, th) {}
	inline MoveNet_(const char* model, Optimization opt = Optimization::DEFAULT, size_t th = default_threading) :
		MoveNet_({{model, opt}}, th) {}
	MoveNet_(std::initializer_list<std::pair<const char*, Optimization> >, size_t th = default_threading);

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
typedef MoveNet_<>	MoveNet;










/** tfmodel.inc **/

template<class derived_t>
AxonRunner_<derived_t>::AxonRunner_(std::initializer_list<std::pair<const char*, Optimization> > options, const char* map, size_t th) :
	vs2::VPipeline<This_t>("Axon Runner "), TfModel(options, th)
{
	::loadObjectLabels(map, this->obj_labels);
	if(this->isValidBuffer() && this->model->outputs().size() == 4) {
		this->coords = reinterpret_cast<float*>(this->model->output_tensor(static_cast<size_t>(OutputTensor::BOX_COORDS))->data.data);
		this->labels = reinterpret_cast<float*>(this->model->output_tensor(static_cast<size_t>(OutputTensor::LABEL_IDX))->data.data);
		this->confidence = reinterpret_cast<float*>(this->model->output_tensor(static_cast<size_t>(OutputTensor::CONFIDENCE))->data.data);
		this->detections = reinterpret_cast<float*>(this->model->output_tensor(static_cast<size_t>(OutputTensor::DETECTIONS))->data.data);
	}
}

template<class derived_t>
void AxonRunner_<derived_t>::process(cv::Mat& io_frame) {
	if(this->isValidBuffer()) {
		cv::resize(io_frame, this->input_tensor, this->input_size);
		this->model->Invoke();

		size_t detected = this->getDetections();
		cv::Rect2f bb;
		for(size_t i = 0; i < detected; i++) {
			bb = std::move(cv::Rect2f(
				cv::Point2f(
					getX1(i) * io_frame.size().width,
					getY1(i) * io_frame.size().height
				),
				cv::Point2f(
					getX2(i) * io_frame.size().width,
					getY2(i) * io_frame.size().height
				)
			));
			cv::rectangle(io_frame, bb, {0, 100, 255}, 4);
			cv::putText(
				io_frame, this->getLabel(i) + ": " + std::to_string(this->getConfidence(i)) + "%",
				cv::Point(bb.tl().x, bb.tl().y - 10), cv::FONT_HERSHEY_DUPLEX, 0.55, {0, 100, 255}, 1, cv::LINE_AA
			);
		}
	}
}

template<class derived_t>
MoveNet_<derived_t>::MoveNet_(std::initializer_list<std::pair<const char*, Optimization> > ops, size_t th) :
	vs2::VPipeline<This_t>("MoveNet Runner "), TfModel(ops, th)
{
	if(this->model->outputs().size() == 1) {
		this->outputs = reinterpret_cast<float*>(this->model->output_tensor(0)->data.data);
	}
}

template<class derived_t>
void MoveNet_<derived_t>::process(cv::Mat& io_frame) {
	if(this->isValidBuffer() && this->isValidOutput()) {
		cv::resize(io_frame, this->input_tensor, this->input_size);
		this->model->Invoke();

		for(size_t i = 0; i < ~Output::TOTAL; i++) {
			this->points[i] = std::move(cv::Point2f(
				this->getX(i) * io_frame.size().width,
				this->getY(i) * io_frame.size().height
			));
			if(this->getConfidence(i) > confid_thresh) {
				cv::circle(io_frame, this->points[i], 1, {25, 255, 0}, 2, cv::LINE_AA);
			}
		}
		for(size_t i = 0; i < connections.size(); i++) {
			if(this->getConfidence(~connections[i].first) > confid_thresh && this->getConfidence(~connections[i].second) > confid_thresh) {
				cv::line(io_frame, this->points[~connections[i].first], this->points[~connections[i].second], {25, 255, 0});
			}
		}
	}
}

#else
#define __TFMODEL_UNSUPPORTED
#endif