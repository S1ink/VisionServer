#include "vision.h"

cs::CvSink switchedCameraVision(const std::vector<VisionCamera>& cameras, std::shared_ptr<nt::NetworkTable> table) {
	if(!table->ContainsKey("Vision Camera Index")) {
		table->PutNumber("Vision Camera Index", 0);
	}
	if(!table->ContainsKey("Vision Cameras Available")) {
		table->PutNumber("Vision Cameras Available", cameras.size());
	}
	static cs::CvSink source;
	if(cameras.size() > 0) {
		source = cameras[0].getVideo();
	}
	table->GetEntry("Vision Camera Index").AddListener(
		[&cameras](const nt::EntryNotification& event) {
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
	return source;
} 

void addNetTableVar(bool& var, const wpi::Twine& name, std::shared_ptr<nt::NetworkTable> table) {
	if(!table->ContainsKey(name)) {
		table->PutBoolean(name.str(), var);
	} else {}
	table->GetEntry(name).AddListener(
		[&var](const nt::EntryNotification& event){
			if(event.value->IsBoolean()) {
				var = event.value->GetBoolean();
				//std::cout << " Networktable var(bool) updated to : " << var << newline;
			}
		},
		NT_NOTIFY_IMMEDIATE | NT_NOTIFY_NEW | NT_NOTIFY_UPDATE
	);
}