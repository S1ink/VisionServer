#include "tfmodel.h"

#ifndef EXCLUDE_TFLITE


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

#endif