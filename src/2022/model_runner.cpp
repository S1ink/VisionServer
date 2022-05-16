#include "model_runner.h"

#include <fstream>
#include <sstream>

#include "../core/vision.h"


void ModelRunner::loadLabels(const std::string& fn, std::vector<std::string>& labels) {
	std::ifstream file(fn);
	std::string line;
	while(std::getline(file, line, '\n')) {
		std::stringstream l(line);
		if(!std::getline(l, line, '"')) {
			std::getline(l, line, '"');
			labels.push_back(line);
		}
	}
}

ModelRunner::ModelRunner(size_t threads) :
	ModelRunner(default_model, default_labels, threads)
{}
ModelRunner::ModelRunner(const char* model, const char* map, size_t threads) :
	VPipeline("TfLite Model Runner "),
	model(tflite::FlatBufferModel::BuildFromFile(model)), builder(*this->model, this->resolver)
{
	// checks
	this->builder.SetNumThreads(threads);
	this->network_impl->AllocateTensors();
	this->input_tensor_idx = this->network_impl->inputs()[0];
	TfLiteIntArray* arr = network_impl->tensor(this->input_tensor_idx)->dims;
	this->input_size = cv::Size(arr->data[1], arr->data[2]);
	this->tensor_buff = cv::Mat(this->input_size, CV_8UC3, this->network_impl->typed_tensor<uint8_t>(this->input_tensor_idx));
	this->bbox_tensor_idx = this->network_impl->outputs()[0];
	this->lab_tensor_idx = this->network_impl->outputs()[1];
	this->per_tensor_idx = this->network_impl->outputs()[2];
	this->size_tensor_idx = this->network_impl->outputs()[3];
	loadLabels(map, this->labels);
}

void ModelRunner::process(cv::Mat& io_frame) {
	cv::resize(io_frame, this->tensor_buff, this->input_size);
	//this->rescale = cv::Size2f((float)this->input_size.width / io_frame.size().width, (float)this->input_size.height / io_frame.size().height);
	this->rescale = ::operator/<int, int, float>(this->input_size, io_frame.size());
	this->network_impl->Invoke();
	float* data = this->network_impl->typed_output_tensor<float>(0);
	cv::Rect2f bb(
		cv::Point2f(
			data[0] * this->rescale.width,
			data[1] * this->rescale.height
		),
		cv::Point2f(
			data[2] * this->rescale.width,
			data[3] * this->rescale.height
		)
	);
	cv::rectangle(io_frame, bb, cv::Scalar(25, 255, 0), 4);
}