#include "VisionServer.h"

const std::shared_ptr<nt::NetworkTable>
	VisionServer::root = nt::NetworkTableInstance::GetDefault().GetTable("Vision Server"),
	VisionServer::targets = nt::NetworkTableInstance::GetDefault().GetTable("Targets"),
	VisionServer::cameras = root->GetSubTable("Cameras"),
	VisionServer::pipelines = root->GetSubTable("Pipelines");
nt::NetworkTableEntry
	VisionServer::active_target = root->GetEntry("Active Target"),
	VisionServer::num_cams = root->GetEntry("Cameras Available"),
	VisionServer::cam_idx = root->GetEntry("Camera Index"),
	VisionServer::num_pipes = root->GetEntry("Pipelines Available"),
	VisionServer::pipe_idx = root->GetEntry("Pipeline Index");

VisionServer::VisionServer() {
	this->updateCameras();
	this->updatePipelines();
}
VisionServer& VisionServer::Get() {
	static VisionServer global;
	return global;
}
void VisionServer::updateCameras() {
	this->vscameras.clear();
	for(const std::string& subtable : cameras->GetSubTables()) {
		this->vscameras.emplace_back(subtable);
	}
}
void VisionServer::updatePipelines() {
	this->vspipelines.clear();
	for(const std::string& subtable : cameras->GetSubTables()) {
		this->vspipelines.emplace_back(subtable);
	}
}

bool VisionServer::incrementCamera() {
	int8_t idx = this->getCameraIdx();
	if(idx + 1 < this->numCameras()) {
		cam_idx.SetDouble(idx + 1);
		return true;
	}
	cam_idx.SetDouble(0.f);
	return false;
}
bool VisionServer::decrementCamera() {
	int8_t idx = this->getCameraIdx();
	if(idx - 1 >= 0) {
		cam_idx.SetDouble(idx - 1);
		return true;
	}
	cam_idx.SetDouble(this->numCameras() - 1);
	return true;
}
bool VisionServer::incrementCamera() {
	int8_t idx = this->getPipelineIdx();
	if(idx + 1 < this->numPipelines()) {
		pipe_idx.SetDouble(idx + 1);
		return true;
	}
	pipe_idx.SetDouble(0.f);
	return false;
}
bool VisionServer::decrementCamera() {
	int8_t idx = this->getPipelineIdx();
	if(idx - 1 >= 0) {
		pipe_idx.SetDouble(idx - 1);
		return true;
	}
	pipe_idx.SetDouble(this->numPipelines() - 1);
	return false;
}

VisionServer::TargetOffset::TargetOffset(double x, double y, double z) : x(x), y(y), z(z) {}
VisionServer::TargetOffset::TargetOffset(const std::shared_ptr<nt::NetworkTable>& target) :
	x(target->GetEntry("x").GetDouble(0.f)), y(target->GetEntry("y").GetDouble(0.f)), z(target->GetEntry("z").GetDouble(0.f)) {}
VisionServer::TargetData::TargetData(double x, double y, double z, double d, double ud, double lr) : 
	pos(x, y, z), distance(d), ud(ud), lr(lr) {}
VisionServer::TargetData::TargetData(const TargetOffset& pos, double d, double ud, double lr) :
	pos(pos), distance(d), ud(ud), lr(lr) {}
VisionServer::TargetData::TargetData(const std::shared_ptr<nt::NetworkTable>& target) :
	pos(target), distance(target->GetEntry("distance").GetDouble(0.f)), 
	ud(target->GetEntry("up-down").GetDouble(0.f)), lr(target->GetEntry("left-right").GetDouble(0.f)) 
{}

VisionServer::VsCamera::VsCamera(const std::shared_ptr<nt::NetworkTable>& nt) {
	this->update(nt);
}
VisionServer::VsCamera::VsCamera(const std::string& name) {
	this->update(name);
}
void VisionServer::VsCamera::update(const std::shared_ptr<nt::NetworkTable>& nt) {
	this->self = nt;
	this->name = nt::NetworkTable::BasenameKey(nt->GetPath());
}
void VisionServer::VsCamera::update(const std::string& tname) {
	this->self = VisionServer::cameras->GetSubTable(tname);
	this->name = tname;
}

VisionServer::VsPipeline::VsPipeline(const std::shared_ptr<nt::NetworkTable>& nt) {
	this->update(nt);
}
VisionServer::VsPipeline::VsPipeline(const std::string& name) {
	this->update(name);
}
void VisionServer::VsPipeline::update(const std::shared_ptr<nt::NetworkTable>& nt) {
	this->self = nt;
	this->name = nt::NetworkTable::BasenameKey(nt->GetPath());
}
void VisionServer::VsPipeline::update(const std::string& tname) {
	this->self = VisionServer::cameras->GetSubTable(tname);
	this->name = tname;
}
std::vector<nt::NetworkTableEntry> VisionServer::VsPipeline::getEntries() const {
	std::vector<nt::NetworkTableEntry> entries;
	for(const std::string& key : this->self->GetKeys()) {
		entries.emplace_back(std::move(this->self->GetEntry(key)));
	}
	return entries;
}
nt::NetworkTableEntry VisionServer::VsPipeline::searchEntries(const std::string& segment) const {
	for(const std::string& key : this->self->GetKeys()) {
		if(key.find(segment) != std::string::npos) {
			return this->self->GetEntry(key);
		}
	}
	return nt::NetworkTableEntry();
}
std::vector<nt::NetworkTableEntry> VisionServer::VsPipeline::searchEntries(const std::vector<std::string>& segments) const {
	std::vector<nt::NetworkTableEntry> entries;
	for(const std::string& key : this->self->GetKeys()) {
		for(size_t i = 0; i < segments.size(); i++) {
			if(key.find(segments[i]) != std::string::npos) {
				entries.emplace_back(std::move(this->self->GetEntry(key)));
			}
		}
	}
	return entries;
}
void VisionServer::VsPipeline::searchUsableEntries() {
	for(const std::string& key : this->self->GetKeys()) {
		if(key.find("Debug")) {
			this->debug = this->self->GetEntry(key);
		}
		if(key.find("Threshold")) {
			this->debug = this->self->GetEntry(key);
		}
	}
}
bool VisionServer::VsPipeline::hasDebug() {
	if(!this->debug.Exists()) {
		this->searchUsableEntries();
		if(!this->debug.Exists()) {
			return false;
		}
	}
	return true;
}
bool VisionServer::VsPipeline::hasThreshold() {
	if(!this->thresh.Exists()) {
		this->searchUsableEntries();
		if(!this->thresh.Exists()) {
			return false;
		}
	}
	return true;
}
bool VisionServer::VsPipeline::setDebug(bool val) {
	if(!this->debug.Exists()) {
		this->searchUsableEntries();
		if(!this->debug.Exists()) {
			return false;
		}
	}
	return this->debug.SetBoolean(val);
}
bool VisionServer::VsPipeline::setThreshold(bool val) {
	if(!this->thresh.Exists()) {
		this->searchUsableEntries();
		if(!this->thresh.Exists()) {
			return false;
		}
	}
	return this->thresh.SetBoolean(val);
}