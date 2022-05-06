#pragma once

#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <chrono>

#include <opencv2/opencv.hpp>

#include <networktables/NetworkTable.h>
#include <cameraserver/CameraServer.h>

#include "visioncamera.h"


class VisionServer2 final {
public:
	inline static const std::shared_ptr<nt::NetworkTable>
		ntable{nt::NetworkTableInstance::GetDefault().GetTable("Vision Server")};;

	inline static VisionServer2& getInstance() {
		static VisionServer2 instance;
		return instance;
	}


	class BasePipe : public cs::CvSource {
		friend class VisionServer2;
	public:
		inline static const std::shared_ptr<nt::NetworkTable> pipe_table{VisionServer2::ntable->GetSubTable("Pipelines")};

	protected:
		inline BasePipe(const char* name) :	/* start enable/disable callback for more efficent threading */
			CvSource(name, cs::VideoMode()), name(name),
			input(this->name), table(pipe_table->GetSubTable(this->name)) {}
		inline BasePipe(const std::string& name) :
			CvSource(name, cs::VideoMode()), name(name),
			input(this->name), table(pipe_table->GetSubTable(this->name)) {}
		inline BasePipe(std::string&& name) :
			CvSource(name, cs::VideoMode()), name(name),
			input(this->name), table(pipe_table->GetSubTable(this->name)) {}
		BasePipe() = delete;


		inline const std::string& getName() const { return this->name; }
		inline const std::shared_ptr<nt::NetworkTable> getTable() const { return this->table; }

		virtual void process(cv::Mat& io_frame) = 0;

		void setCamera(const VisionCamera&);

	private:
		std::string name;
		cs::CvSink input;
		const std::shared_ptr<nt::NetworkTable> table;


	};


	static void addCamera(VisionCamera&&);
	static void addCameras(std::vector<VisionCamera>&&);
	static void setCameras(std::vector<VisionCamera>&&);
	static const std::vector<VisionCamera>& getCameras();
	static size_t numCameras();

	template<class pipeline>
	static void addPipeline();
	//static void addPipeline(BasePipe*);
	template<class... pipelines>
	static void addPipelines();
	//static void addPipelines(std::initializer_list<BasePipe*>);
	template<class... pipelines>
	static void setPipelines();
	//static void setPipelines(std::initializer_list<BasePipe*>);
	static const std::vector<std::unique_ptr<BasePipe> >& getPipelines();
	static size_t numPipelines();

	static void addStream();
	static void addStream(std::string_view);
	static void addStream(std::string_view, int port);
	static void addStreams(size_t = 2);
	//static const std::vector<OutputStream>& getStreams();
	static size_t numStreams();

	static bool run(uint16_t rps = 50);
	static bool runThread(uint16_t rps = 50);
	static bool isRunning();
	static bool stop();

	static void test();


protected:
	inline static VisionServer2& inst() { return getInstance(); }

	static void pipelineRunner(BasePipe*, uint16_t rps);

	template<class pipeline = void, class... pipelines>
	static void pipeExpander(std::vector<std::unique_ptr<BasePipe> >&);

	//inline static void nullPipeDeleter(BasePipe*) {}


private:
	VisionServer2();
	~VisionServer2();
	VisionServer2(const VisionServer2&) = delete;
	VisionServer2(VisionServer2&&) = delete;
	void operator=(const VisionServer2&) = delete;
	void operator=(VisionServer2&&) = delete;

	std::vector<VisionCamera> cameras;
	std::vector<std::unique_ptr<BasePipe> > pipelines;

	std::thread head;
	std::atomic<bool> is_running{false};

	// struct FrameMutex {
	// public:
	// 	inline FrameMutex(cv::Mat&& mat) : buffer(mat) {}
	// 	inline FrameMutex(cv::Size s, int t) : buffer(s, t) {}

	// 	bool update(const VisionCamera& cam, double timeo);
	// 	void transfer(cv::Mat&);
	// 	void clone(cv::Mat&);

	// 	inline int test() { return this->buffer.cols; }

	// private:
	// 	std::unique_lock<std::mutex> access;
	// 	cv::Mat buffer;

	// };
	// std::vector<FrameMutex> framebuffer;

	struct OutputStream {
		friend class VisionServer2;
	public:
		inline static const std::shared_ptr<nt::NetworkTable> streams_table = ntable->GetSubTable("Streams");

		OutputStream(cs::MjpegServer&& s);	// setup ntable values
		~OutputStream() = default;	// remove ntable values

	private:
		cs::MjpegServer server;
		const std::shared_ptr<nt::NetworkTable> table;

	};
	std::vector<OutputStream> streams;


};



template<class derived>	// CRTP for instance counting -> all derived classes should inherit a passthrough template if they are expected to be extended as well
class VPipeline2 : public VisionServer2::BasePipe {
	typedef struct VPipeline2<derived>		This_t;		// "this type"
	//static_assert(std::is_base_of<This_t, derived>::value, "[template] Class is not derived from VPipeline2<Class> - CRTP is necessary.");

public:
	VPipeline2() = delete;
	VPipeline2(const VPipeline2&) = delete;
	inline VPipeline2(const char* name) : 
		BasePipe(name + std::to_string(This_t::instances + 1)), instance(This_t::instances + 1)
			{ This_t::instances++; }
	inline VPipeline2(const std::string& name) :
		BasePipe(name + std::to_string(This_t::instances + 1)), instance(This_t::instances + 1)
			{ This_t::instances++; }
	inline VPipeline2(std::string&& name) :
		BasePipe(name + std::to_string(This_t::instances + 1)), instance(This_t::instances + 1)
			{ This_t::instances++; }
	inline virtual ~VPipeline2() { instances--; }

	inline static uint32_t getInstances() { return This_t::instances; }
	template<class derived_t>
	inline static uint32_t getTypeInstances() {
		static_assert(std::is_base_of<VPipeline2<derived_t>, derived_t>::value, "[template] Class is not derived from VPipeline2<Class> - CRTP is necessary.");
		return VPipeline2<derived_t>::instances;
	}

	virtual void process(cv::Mat& io_frame) override;


private:
	inline static std::atomic<uint32_t> instances{0};
	uint32_t instance;


};


#include "visionserver2.inc"