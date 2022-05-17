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


/** Runs a {Axon-generated} TensorFlow Lite machine-learning model and outputs bouding boxes */
class ModelRunner : public vs2::VPipeline<ModelRunner> {
public:
	inline static const char
		*default_model = "unoptimized.tflite",
		*edge_model = "model.tflite",
		*default_labels = "map.pbtxt"
	;

	ModelRunner(size_t threads = std::thread::hardware_concurrency() / 2);
	ModelRunner(const char* model, const char* map, size_t threads = std::thread::hardware_concurrency() / 2);

	virtual void process(cv::Mat& io_frame) override;


	static void loadLabels(const std::string& fn, std::vector<std::string>& labels);

protected:
	std::unique_ptr<tflite::FlatBufferModel> model;
	std::unique_ptr<tflite::Interpreter> network;
	const tflite::ops::builtin::BuiltinOpResolver resolver;

	std::vector<std::string> labels;

	cv::Mat tensor_buff;
	cv::Size input_size;
	TfLiteTensor
		*input_tensor,		// the image input (300x300x3C)
		*coords_tensor,		// the bounding box coord output
		*labels_tensor,		// the label id output
		*confidence_tensor,	// the confidence output
		*size_tensor		// num of detections output
	;

	enum class OutputTensors {
		BOX_COORDS = 0,
		LABEL_IDX = 1,
		CONFIDENCE = 2,
		NUM_OUTPUTS = 3
	};


};