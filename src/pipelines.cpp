#include "pipelines.h"
#include "vision.h"

TestPipeline::TestPipeline(const VisionServer* server) : PipelineBase(server) {
	addNetTableVar(this->weight, "Weight (Red and Green)", this->table);
	addNetTableVar(this->threshold, "Threshold (For weighted)", this->table);
	//addNetTableVar(this->options[0], "Morphological Operations", this->table);

	this->resizeBuffers(this->env->getCurrentResolution());
}

void TestPipeline::resizeBuffers(cv::Size size) {
	this->buffer = cv::Mat(size/this->scale, CV_8UC3);
	this->binary = cv::Mat(size/this->scale, CV_8UC1);
	for(size_t i = 0; i < this->channels.size(); i++) {
		channels[i] = cv::Mat(size/this->scale, CV_8UC1);
	}
}

void TestPipeline::process(cv::Mat& frame, cs::CvSource& output, bool show_binary) {
	this->beg = CHRONO::high_resolution_clock::now();

	if(cmpSize(frame.size())) {
		this->resizeBuffers(frame.size());
	}

	cv::resize(frame, this->buffer, cv::Size(), 1.0/this->scale, 1.0/this->scale);
	cv::split(this->buffer, this->channels);
	cv::addWeighted(this->channels[1], this->weight, this->channels[2], this->weight, 0.0, this->binary);
	cv::subtract(this->channels[0], this->binary, this->binary);
	memcpy_threshold_binary_asm(this->binary.data, this->binary.data, this->binary.size().area(), this->threshold);

	// if(options[0]) {
	// 	cv::erode(this->binary, this->binary, cv::getStructuringElement(cv::MORPH_ERODE, cv::Size(3, 3)));
	// 	cv::dilate(this->binary, this->binary, cv::getStructuringElement(cv::MORPH_DILATE, cv::Size(3, 3)));
	// }

	if(show_binary) {
		cv::cvtColor(this->binary, this->buffer, cv::COLOR_GRAY2BGR, 3);
		cv::resize(this->buffer, frame, cv::Size(), this->scale, this->scale, cv::INTER_NEAREST);
	} else {
		this->contours.clear();
		this->poly_contour.clear();
		this->largest = 0.f;
		this->target = -1;

		cv::findContours(this->binary, this->contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

		for(size_t i = 0; i < this->contours.size(); i++) {
			this->area = cv::contourArea(this->contours[i]);
			if(this->area > this->largest) {
				this->largest = this->area;
				this->target = i;
			}
		}

		if(this->target >= 0) {
			this->boundingbox = cv::boundingRect(this->contours[this->target]);
			cv::Point2i tl = this->boundingbox.tl(), br = this->boundingbox.br();
			cv::rectangle(frame, cv::Point2i(tl.x*this->scale, tl.y*this->scale), cv::Point(br.x*this->scale, br.y*this->scale), cv::Scalar(255, 0, 0), 2);
		} else {
			cv::putText(
				frame, "NO TARGETS DETECTED", 
				cv::Point(frame.size().width*0.2, frame.size().height*0.5), 
				cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 255), 2, cv::LINE_AA
			);

			// DEBUG
			// cv::putText(
			// 	frame, "CONTOURS: " + std::to_string(this->contours.size()), 
			// 	cv::Point(frame.size().width*0.2, frame.size().height*0.5), 
			// 	cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 255), 2, cv::LINE_AA
			// );
		}
	}

	cv::putText(
		frame, "CPU: " + std::to_string(CPU::get().refPercent()*100.f) + "% | " + std::to_string(CPU::temp()) + "*C", 
		cv::Point(0, 15), 
		cv::FONT_HERSHEY_DUPLEX, 0.45, cv::Scalar(0, 255, 0), 1, cv::LINE_AA
	);
	cv::putText(
		frame, "Frametime: " + std::to_string(this->frame_time*1000) + " ms", 
		cv::Point(0, 30), 
		cv::FONT_HERSHEY_DUPLEX, 0.45, cv::Scalar(0, 255, 0), 1, cv::LINE_AA
	);
	cv::putText(
		frame, "Looptime: " + std::to_string(this->loop_time*1000) + " ms", 
		cv::Point(0, 45), 
		cv::FONT_HERSHEY_DUPLEX, 0.45, cv::Scalar(0, 255, 0), 1, cv::LINE_AA
	);
	cv::putText(
		frame, "FPS (1F): " + std::to_string(this->fps), 
		cv::Point(0, 60), 
		cv::FONT_HERSHEY_DUPLEX, 0.45, cv::Scalar(0, 255, 0), 1, cv::LINE_AA
	);
	cv::putText(
		frame, "FPS (1S): " + std::to_string(this->fps_1s), 
		cv::Point(0, 75), 
		cv::FONT_HERSHEY_DUPLEX, 0.45, cv::Scalar(0, 255, 0), 1, cv::LINE_AA
	);
	cv::putText(
		frame, "Total Frames: " + std::to_string(this->total_frames), 
		cv::Point(0, 90), 
		cv::FONT_HERSHEY_DUPLEX, 0.45, cv::Scalar(0, 255, 0), 1, cv::LINE_AA
	);

	output.PutFrame(frame);
	this->end = CHRONO::high_resolution_clock::now();
	this->total_frames++;

	this->total_time = CHRONO::duration<double>(this->end - this->getEnvStart()).count();
	this->frame_time = CHRONO::duration<double>(this->end - this->beg).count();
	this->loop_time = CHRONO::duration<double>(this->end - this->last).count();
	this->last = this->end;

	this->fps = 1.f/this->loop_time;
	if ((int)this->total_time > (int)this->sec1_time) {
		this->fps_1s = ((this->total_frames - this->sec1_frames) / (this->total_time - this->sec1_time));
		sec1_time = total_time;
		sec1_frames = total_frames;
	}
}