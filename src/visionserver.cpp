#include "visionserver.h"

PipelineBase::PipelineBase(VisionServer* server) : env(server) {}

void PipelineBase::process(cv::Mat& frame, cs::CvSource& output) {
    output.PutFrame(frame);
}

std::vector<VisionCamera>* PipelineBase::getEnvCameras() {
	return this->env->cameras;
}
cs::CvSink PipelineBase::getEnvSource() {
	return this->env->source;
}
cs::CvSource PipelineBase::getEnvStream() {
	return this->env->stream;
}
CHRONO::high_resolution_clock::time_point PipelineBase::getEnvStart() {
	return this->env->start;
}

VisionServer::VisionServer(std::vector<VisionCamera>& cameras) : cameras(&cameras) {
	for(size_t i = 0; i < cameras.size(); i++) {
		cameras[i].setNetworkAdjustable();
	}

    if(!this->table->ContainsKey("Camera Index")) {
		this->table->PutNumber("Camera Index", 0);
	}
	if(!this->table->ContainsKey("Cameras Available")) {
		this->table->PutNumber("Cameras Available", cameras.size());
	}
	//if(cameras.size() > 0) {	// config parser should prevent this from happening
		this->source = cameras[0].getVideo();
		this->stream = cameras[0].getSeparateServer();
	//}
	table->GetEntry("Vision Camera Index").AddListener(
		[&cameras, this](const nt::EntryNotification& event) {
			if(event.value->IsDouble()) {
				size_t idx = event.value->GetDouble();
				if(idx >= 0 && idx < cameras.size()) {
					source.SetSource(cameras[idx]);
					source.SetConfigJson(cameras[idx].getStreamJson());
				}
			}
		},
		NT_NOTIFY_IMMEDIATE | NT_NOTIFY_NEW | NT_NOTIFY_UPDATE
	);
}

size_t VisionServer::validIndexes() {
    if(this->cameras) {
        return this->cameras->size();
    }
    return 0;
}
bool VisionServer::setCamera(size_t idx) {
    if(idx < this->validIndexes()) {
        //this->source.SetSource(this->cameras->operator[](idx));   // this should be taken care of by the networktables callback
        this->table->GetEntry("Camera Index").SetDouble(idx);
    }
    return false;
}
cv::Size VisionServer::getCurrentResolution() {
	return getResolution(this->source.GetSource().GetVideoMode());
}