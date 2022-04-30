#include "visionserver2.h"


void VisionServer::framebuffWorker() {
	uint16_t max_fps = 1;
	std::chrono::high_resolution_clock::time_point tbuff;
	for(size_t i = 0; i < instance.cameras.size(); i++) {
		int fps = instance.cameras.at(i).getConfigFPS();
		max_fps = fps > max_fps ? fps : max_fps;
	}
	float max_millis = 1000.f / max_fps;
	for(size_t i = instance.framebuffer.size(); i < instance.cameras.size(); i++) {
		instance.framebuffer.emplace_back(instance.cameras.at(i).getResolution(), CV_8UC3);
	}
	double timeout = (max_millis - 5.f) / instance.cameras.size() * 1000.0;

	while(instance.is_running) {
		tbuff = std::chrono::high_resolution_clock::now();
		for(size_t i = 0; i < instance.cameras.size(); i++) {
			instance.framebuffer.at(i).update(instance.cameras.at(i), timeout);
		}
		std::this_thread::sleep_for(
			std::chrono::nanoseconds((uint64_t)(max_millis * 1000000)) - (
				std::chrono::high_resolution_clock::now() - tbuff
			)
		);
	}

}

bool VisionServer::FrameMutex::update(const VisionCamera& cam, double timeo) {
	if(this->access.try_lock()) {
		cam.getFrame(this->buffer, timeo);
		this->access.unlock();
		return true;
	}
	return false;
}
void VisionServer::FrameMutex::transfer(cv::Mat& frame) {
	this->access.lock();
	frame = this->buffer;
	this->access.unlock();
}
void VisionServer::FrameMutex::clone(cv::Mat& frame) {
	this->access.lock();
	this->buffer.copyTo(frame);
	this->access.unlock();
}