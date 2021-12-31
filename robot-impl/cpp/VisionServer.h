#pragma once

//#define COMMANDBASED

#include <networktables/NetworkTable.h>
#include <networktables/NetworkTableEntry.h>
#include <networktables/NetworkTableInstance.h>

#ifdef COMMANDBASED
#include <frc2/command/CommandHelper.h>
#include <frc2/command/CommandBase.h>
#endif

#include <vector>
#include <string>

class VisionServer {
public:

	struct TargetOffset;
	struct TargetData;
	class VsCamera;
	class VsPipeline;

	static const std::shared_ptr<nt::NetworkTable> root, targets, cameras, pipelines;
	static nt::NetworkTableEntry active_target, num_cams, cam_idx, num_pipes, pipe_idx;

	static VisionServer& Get();

	void updateCameras();
	void updatePipelines();
	inline std::vector<VsCamera>& getCameras() { return this->vscameras; }
	inline std::vector<VsPipeline>& getPipelines() { return this->vspipelines; }
	inline VsCamera* getCamera(size_t idx) { return idx < this->vscameras.size() ? &this->vscameras[idx] : nullptr; }	// returns nullptr on oob error
	inline VsPipeline* getPipeline(size_t idx) { return idx < this->vspipelines.size() ? &this->vspipelines[idx] : nullptr; }	// returns nullptr on oob error
	inline VsCamera& getCurrentCamera() { return *this->getCamera(this->getCameraIdx()); }	// could possibly segfault if the index isn't up to date, but this is unlikely
	inline VsPipeline& getCurrentPipeline() { return *this->getPipeline(this->getPipelineIdx()); }	//^

	inline bool isShowingStatistics() const { return root->GetEntry("Show Statistics").GetBoolean(false); }
	inline void setStatistics(bool val) { root->GetEntry("Show Statistics").SetBoolean(val); }
	inline void toggleStatistics() { this->setStatistics(!this->isShowingStatistics()); }

	inline bool hasActiveTarget() const { return active_target.GetString("none") != "none"; }
	inline std::shared_ptr<nt::NetworkTable> getActiveTarget() const { return targets->GetSubTable(active_target.GetString("none")); }
	inline std::string getActveTargetName() const { return active_target.GetString("none"); }
	inline double getDistance() const { return this->getActiveTarget()->GetEntry("distance").GetDouble(0.f); }
	inline double getThetaUD() const { return this->getActiveTarget()->GetEntry("up-down").GetDouble(0.f); }
	inline double getThetaLR() const { return this->getActiveTarget()->GetEntry("left-right").GetDouble(0.f); }
	inline TargetOffset getTargetPos() const { return TargetOffset(this->getActiveTarget()); }
	inline TargetData getTargetData() const { return TargetData(this->getActiveTarget()); }

	inline uint8_t numCameras() const { return num_cams.GetDouble(0.f); }
	inline int8_t getCameraIdx() const { return cam_idx.GetDouble(-1.f); }
	inline bool setCamera(uint8_t idx) { return idx < this->numCameras() && cam_idx.SetDouble(idx); }
	bool incrementCamera();
	bool decrementCamera();

	inline uint8_t numPipelines() const { return num_pipes.GetDouble(0.f); }
	inline int8_t getPipelineIdx() const { return pipe_idx.GetDouble(-1.f); }
	inline bool setPipeline(uint8_t idx) { return idx < this->numPipelines() && pipe_idx.SetDouble(idx); }
	bool incrementPipeline();
	bool decrementPipeline();

	struct TargetOffset {
		double x, y, z;

		TargetOffset(double x, double y, double z);
		TargetOffset(const std::shared_ptr<nt::NetworkTable>& target);
	};
	struct TargetData {
		TargetOffset pos;
		double distance, ud, lr;

		TargetData(double x, double y, double z, double d, double ud, double lr);
		TargetData(const TargetOffset& pos, double d, double ud, double lr);
		TargetData(const std::shared_ptr<nt::NetworkTable>& target);
	};

	class VsCamera {
	public:
		void update(const std::shared_ptr<nt::NetworkTable>& nt);
		void update(const std::string& tname);

		VsCamera(const std::shared_ptr<nt::NetworkTable>& nt);
		VsCamera(const std::string& name);

		inline const std::shared_ptr<nt::NetworkTable>& get() const { return this->self; }
		inline const std::string& getName() const { return this->name; }

		inline int8_t getExposure() const { return this->self->GetEntry("Exposure").GetDouble(0.f); }
		inline int8_t getBrightness() const { return this->self->GetEntry("Brightness").GetDouble(0.f); }
		inline int16_t getWhiteBalance() const { return this->self->GetEntry("WhiteBalance").GetDouble(0.f); }
		inline bool setExposure(int8_t e) { return this->self->GetEntry("Exposure").SetDouble(e); }
		inline bool setBrightness(int8_t b) { return this->self->GetEntry("Brightness").SetDouble(b); }
		inline bool setWhiteBalance(int16_t wb) { return this->self->GetEntry("WhiteBalance").SetDouble(wb); }
		inline nt::NetworkTableEntry getExposureEntry() const { return this->self->GetEntry("Exposure"); }
		inline nt::NetworkTableEntry getBrightnessEntry() const { return this->self->GetEntry("Brightness"); }
		inline nt::NetworkTableEntry getWhiteBalanceEntry() const { return this->self->GetEntry("WhiteBalance"); }

	private:
		std::shared_ptr<nt::NetworkTable> self;
		std::string name;

	};
	class VsPipeline {
	public:
		void update(const std::shared_ptr<nt::NetworkTable>& nt);
		void update(const std::string& tname);

		VsPipeline(const std::shared_ptr<nt::NetworkTable>& nt);
		VsPipeline(const std::string& name);

		inline const std::shared_ptr<nt::NetworkTable>& get() const { return this->self; }
		inline const std::string& getName() const { return this->name; }

		std::vector<nt::NetworkTableEntry> getEntries() const;
		nt::NetworkTableEntry searchEntries(const std::string& segment) const;
		std::vector<nt::NetworkTableEntry> searchEntries(const std::vector<std::string>& segments) const;
		void searchUsableEntries();
		bool hasDebug();
		bool hasThreshold();
		bool setDebug(bool val);
		bool setThreshold(bool val);

	private:
		std::shared_ptr<nt::NetworkTable> self;
		std::string name;
		nt::NetworkTableEntry debug, thresh;

	};

private:
	VisionServer();

	std::vector<VsCamera> vscameras;
	std::vector<VsPipeline> vspipelines;

#ifdef COMMANDBASED
	class IncrementCamera : public frc2::CommandHelper<frc2::CommandBase, IncrementCamera> {
	public:
		IncrementCamera() = default;
		inline void Initialize() override { Get().incrementCamera(); }
		inline bool IsFinished() override { return true; }
	};
	class DecrementCamera : public frc2::CommandHelper<frc2::CommandBase, DecrementCamera> {
	public:
		DecrementCamera() = default;
		inline void Initialize() override { Get().decrementCamera(); }
		inline bool IsFinished() override { return true; }
	};
	class IncrementPipeline : public frc2::CommandHelper<frc2::CommandBase, IncrementPipeline> {
	public:
		IncrementPipeline() = default;
		inline void Initialize() override { Get().incrementPipeline(); }
		inline bool IsFinished() override { return true;}
	};
	class DecrementPipeline : public frc2::CommandHelper<frc2::CommandBase, DecrementPipeline> {
	public:
		DecrementPipeline() = default;
		inline void Initialize() override { Get().decrementPipeline(); }
		inline bool IsFinished() override { return true; }
	};
	class ToggleStatistics : public frc2::CommandHelper<frc2::CommandBase, ToggleStatistics> {
	public:
		ToggleStatistics() = default;
		inline void Initialize() override { Get().toggleStatistics(); }
		inline bool IsFinished() override { return true; }
	};
	//class ToggleDebug : public frc2::CommandBase {};
	//class TogglePipeline : public frc2::CommandBase {};

	static IncrementCamera inc_camera;
	static DecrementCamera dec_camera;
	static IncrementPipeline inc_pipeline;
	static DecrementPipeline dec_pipeline;
	static ToggleStatistics toggle_stats;

public:
	inline static IncrementCamera& getCameraIncrementCommand() { return inc_camera; }
	inline static DecrementCamera& getCameraDecrementCommand() { return dec_camera; }
	inline static IncrementPipeline& getPipelineIncrementCommand() { return inc_pipeline; }
	inline static DecrementPipeline& getPipelineDecrementCommand() { return dec_pipeline; }
	inline static ToggleStatistics& getStatisticsToggleCommand() { return toggle_stats; }
#endif
};