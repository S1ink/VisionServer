#pragma once

#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <chrono>

#include <opencv2/opencv.hpp>

#include <networktables/NetworkTable.h>

#include "visioncamera.h"


class VisionServer final {
public:
	inline static const std::shared_ptr<nt::NetworkTable>
		ntable{nt::NetworkTableInstance::GetDefault().GetTable("Vision Server")},
		cameras{ntable->GetSubTable("Cameras")}
	;

	inline static VisionServer& getInstance() { return VisionServer::instance; }


	class BasePipe {
	public:
		inline static const std::shared_ptr<nt::NetworkTable> base_table{VisionServer::ntable->GetSubTable("Pipelines")};

		inline BasePipe(const char* name) :
			name(name), table(base_table->GetSubTable(this->name)) {/* start callback for enable/disable (threads?) */}
		inline BasePipe(const std::string& name) :
			name(name), table(base_table->GetSubTable(this->name)) {}
		inline BasePipe(std::string&& name) :
			name(name), table(base_table->GetSubTable(this->name)) {}
		BasePipe() = delete;


		inline const std::string& getName() const { return this->name; }
		inline const std::shared_ptr<nt::NetworkTable> getTable() { return this->table; }

		virtual void process(cv::Mat& io_frame) = 0;

	private:
		std::string name;
		cs::CvSource output;
		const std::shared_ptr<nt::NetworkTable> table;


	};


	static void addCamera(VisionCamera&&);
	static void addCameras(std::vector<VisionCamera>&&);
	static void setCameras(std::vector<VisionCamera>&&);
	static const std::vector<VisionCamera>& getCameras();
	static size_t numCameras();

	template<class pipeline>
	static void addPipeline();
	template<class... pipelines>
	static void addPipelines();
	template<class... pipelines>
	static void setPipelines();
	static const std::vector<std::unique_ptr<BasePipe> >& getPipelines();
	static size_t numPipelines();

	static void addStream();
	static void addStream(std::string_view);
	static void addStream(cs::MjpegServer&&);
	static void addStreams(size_t = 2);
	static const std::vector<cs::MjpegServer>& getStreams();
	static size_t numStreams();

	static void run(uint16_t rps = 50);
	static bool isRunning();


protected:
	static void framebuffWorker();	// updates framebuffers from each camera stream


private:
	VisionServer();
	~VisionServer();
	VisionServer(const VisionServer&) = delete;
	VisionServer(VisionServer&&) = delete;
	void operator=(const VisionServer&) = delete;
	void operator=(VisionServer&&) = delete;

	static VisionServer instance;

	std::vector<VisionCamera> cameras;
	std::vector<std::unique_ptr<BasePipe> > pipelines;
	std::vector<cs::MjpegServer> streams;

	std::atomic<bool> is_running{false};
	//std::atomic<uint64_t> clock{0};

	struct FrameMutex {
	public:
		inline FrameMutex(cv::Mat&& mat) : buffer(mat) {}
		inline FrameMutex(cv::Size s, int t) : buffer(s, t) {}

		bool update(const VisionCamera& cam, double timeo);
		void transfer(cv::Mat&);
		void clone(cv::Mat&);

	private:
		std::mutex access;
		cv::Mat buffer;

	};

	std::vector<FrameMutex> framebuffer;


};



template<class derived>	// CRTP for instance counting -> all derived classes should inherit a passthrough template if they are expected to be extended as well
class VPipeline : public VisionServer::BasePipe {
	typedef VPipeline<derived>	This_t;		// "this type"
	static_assert(std::is_base_of<This_t, derived>::value, "[template] Class is not derived from VPipeline<Class> - CRTP is necessary.");

public:
	VPipeline() = delete;
	VPipeline(const VPipeline&) = delete;
	inline VPipeline(const char* name) : 
		BasePipe(name + std::to_string(This_t::instances + 1))
			{ This_t::instances++; }
	inline VPipeline(const std::string& name) :
		BasePipe(name + std::to_string(This_t::instances + 1))
			{ This_t::instances++; }
	inline VPipeline(std::string&& name) :
		BasePipe(name + std::to_string(This_t::instances + 1))
			{ This_t::instances++; }
	inline virtual ~VPipeline() { instances--; }

	inline static uint32_t getInstances() { return This_t::instances; }
	template<class derived>
	inline static uint32_t getTypeInstances() {
		static_assert(std::is_base_of<This_t, derived>::value, "[template] Class is not derived from VPipeline<Class> - CRTP is necessary.");
		return VPipeline<derived>::instances;
	}


private:
	inline static std::atomic<uint32_t> instances{0};


};

#include "visionserver2.inc"