#pragma once

#include <vector>
#include <string>
#include <thread>

#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/interpreter_builder.h>
#include <tensorflow/lite/model_builder.h>
#include <tensorflow/lite/kernels/register.h>
#include <opencv2/opencv.hpp>

#include "../core/visionserver2.h"

#include "tools/src/resources.h"


/** Runs a TensorFlow Lite machine-learning model and outputs bouding boxes */
class ModelRunner : public vs2::VPipeline<ModelRunner> {
public:
	inline static const char* default_model = "unoptimized.tflite";
	inline static const char* edge_model = "model.tflite";
	inline static const char* default_labels = "map.pbtxt";

	static void loadLabels(const std::string& fn, std::vector<std::string>& labels);

	ModelRunner(size_t threads = std::thread::hardware_concurrency() / 2);
	ModelRunner(const char* model, const char* map, size_t threads = std::thread::hardware_concurrency() / 2);

	virtual void process(cv::Mat& io_frame) override;


protected:
	const std::unique_ptr<tflite::FlatBufferModel> model;
	const std::unique_ptr<tflite::Interpreter> network_impl;
	const tflite::ops::builtin::BuiltinOpResolver resolver;
	tflite::InterpreterBuilder builder;

	std::vector<std::string> labels;

	cv::Mat tensor_buff;
	cv::Size2f rescale;
	cv::Size input_size;
	uint32_t
		input_tensor_idx,
		bbox_tensor_idx,
		lab_tensor_idx,
		per_tensor_idx,
		size_tensor_idx
	;


};