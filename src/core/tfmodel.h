#pragma once

#include <vector>
#include <string>

#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/interpreter_builder.h>
#include <tensorflow/lite/model_builder.h>
#include <tensorflow/lite/kernels/register.h>
#include <edgetpu.h>
#include <opencv2/opencv.hpp>

#include "tools/src/resources.h"
#include "visionserver2.h"


using namespace vs2;

class TfModel {
public:


protected:
	std::unique_ptr<tflite::FlatBufferModel> model;
	std::unique_ptr<tflite::Interpreter> nnetwork;
	std::shared_ptr<edgetpu::EdgeTpuContext> edgetpu_context;
	tflite::ops::builtin::BuiltinOpResolver resolver;

	cv::Mat input_tensor;
	cv::Size input_size;


};