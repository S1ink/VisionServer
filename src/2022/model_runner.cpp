#include "model_runner.h"

#include <fstream>
#include <sstream>

#include "../core/vision.h"


void ModelRunner::loadLabels(const std::string& fn, std::vector<std::string>& labels) {
	labels.clear();
	std::ifstream file(fn);
	std::string line;
	int32_t start, end;
	while(std::getline(file, line, '\n')) {
		start = end = -1;
        for (size_t i = 0; i < line.size(); i++) {
            if (line.at(i) == '"') {
                if (start < 0) {
                    start = i + 1;
                }
                else {
                    end = i;
                    i = line.size();	// break
                }
            }
        }
        if (start >= 0 && end >= 0) {
            labels.emplace_back(std::move(line.substr(start, end - start)));
        }
	}
	file.close();
}

ModelRunner::ModelRunner(size_t threads) :
	ModelRunner(default_model, default_labels, threads)
{}
ModelRunner::ModelRunner(const char* model, const char* map, size_t threads) :
	VPipeline("TfLite Model Runner ")
{
	this->model = tflite::FlatBufferModel::BuildFromFile(model);
	if(!model) {
		// model did not load...
	}
	// deal with edge-tpu models...
	tflite::InterpreterBuilder builder(*this->model, this->resolver);
	builder.SetNumThreads(threads);	// return status
	builder(&this->network);		// return status
	this->network->AllocateTensors();
	if(this->network->inputs().size() != 1) {
		// model not compatible...?
	}
	this->input_tensor = this->network->input_tensor(0);
	TfLiteIntArray* arr = this->input_tensor->dims;
	this->input_size = cv::Size(arr->data[1], arr->data[2]);
	if(arr->data[3] != 3) {
		// channels...
	}
	if(this->input_tensor->type != kTfLiteUInt8) {
		// wrong type...
	}
	this->tensor_buff = cv::Mat(this->input_size, CV_8UC3, this->input_tensor->data.data);
	if(this->network->outputs().size() != 4) {
		// outputs not correct...?
	}
	this->coords_tensor = this->network->output_tensor((size_t)OutputTensors::BOX_COORDS);
	this->labels_tensor = this->network->output_tensor((size_t)OutputTensors::LABEL_IDX);
	this->confidence_tensor = this->network->output_tensor((size_t)OutputTensors::CONFIDENCE);
	this->size_tensor = this->network->output_tensor((size_t)OutputTensors::NUM_OUTPUTS);
	loadLabels(map, this->labels);
}

void ModelRunner::process(cv::Mat& io_frame) {
	cv::resize(io_frame, this->tensor_buff, this->input_size);
	this->network->Invoke();	// run the model

	float
		*coords = (float*)this->coords_tensor->data.data,
		*labels = (float*)this->labels_tensor->data.data,
		*confid = (float*)this->confidence_tensor->data.data
	;
	size_t detected = ((float*)this->size_tensor->data.data)[0];
	cv::Rect2f bb;

	for(size_t i = 0; i < detected; i++) {
		bb = cv::Rect2f(
			cv::Point2f(
				coords[i * 4 + 1] * io_frame.size().width,
				coords[i * 4] * io_frame.size().height
			),
			cv::Point2f(
				coords[i * 4 + 3] * io_frame.size().width,
				coords[i * 4 + 2] * io_frame.size().height
			)
		);
		cv::rectangle(io_frame, bb, cv::Scalar(25, 255, 0), 4);
		cv::putText(
			io_frame,
			this->labels.at(labels[i]) + 
			": " + std::to_string((uint32_t)(confid[i] * 100.f)) + "%",
			cv::Point(bb.tl().x, bb.tl().y - 10), cv::FONT_HERSHEY_DUPLEX, 0.55, {10, 255, 0}, 1, cv::LINE_AA
		);
	}

}