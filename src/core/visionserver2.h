#pragma once

#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <chrono>
#include <initializer_list>

#include <opencv2/opencv.hpp>

#include <networktables/NetworkTable.h>
#include <cameraserver/CameraServer.h>

#include "tools/src/types.h"
#include "visioncamera.h"


namespace vs2 {

class VisionServer final {
	struct OutputStream;
public:
	inline static const std::shared_ptr<nt::NetworkTable>
		base_table{nt::NetworkTableInstance::GetDefault().GetTable("Vision Server")};

	inline static void Init() {
		getInstance();
	}
	inline static VisionServer& getInstance() {
		static VisionServer instance;
		return instance;
	}


	class BasePipe : public cs::CvSource {
		friend class VisionServer;
	public:
		inline static const std::shared_ptr<nt::NetworkTable>
			pipe_table{VisionServer::base_table->GetSubTable("Pipelines")};

		inline const std::string& getName() const { return this->name; }
		inline const std::shared_ptr<nt::NetworkTable> getTable() const { return this->table; }

		virtual void process(cv::Mat& io_frame) = 0;

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
	static void addPipeline(BasePipe*);
	template<class... pipelines>
	static void addPipelines();
	static void addPipelines(std::vector<BasePipe*>&&);
	static void addPipelines(std::initializer_list<BasePipe*>);
	template<class... pipelines>
	static void setPipelines();
	static void setPipelines(std::vector<BasePipe*>&&);
	static void setPipelines(std::initializer_list<BasePipe*>);
	static const std::vector<BasePipe*>& getPipelines();
	static size_t numPipelines();

	static void addStream();
	static void addStream(std::string_view);
	static void addStream(std::string_view, int port);
	static void addStreams(size_t = 2);
	static const std::vector<OutputStream>& getStreams();
	static size_t numStreams();

	static void compensate();

	static bool run(uint16_t rps = 50);
	static bool runThread(uint16_t rps = 50);
	static bool isRunning();
	static bool stop();
	static inline void stopExit() { stop(); }

	static void test();


	template<class pipeline = void, class... pipelines>
	static size_t pipeExpander(std::vector<std::unique_ptr<BasePipe> >&);

protected:
	inline static VisionServer& inst() { return getInstance(); }

	static void pipelineRunner(BasePipe*, uint16_t rps);

private:
	VisionServer();
	~VisionServer();
	VisionServer(const VisionServer&) = delete;
	VisionServer(VisionServer&&) = delete;
	void operator=(const VisionServer&) = delete;
	void operator=(VisionServer&&) = delete;

	//inline static VisionServer* _inst{nullptr};

	std::vector<VisionCamera> cameras;
	std::vector<BasePipe*> pipelines;
	std::vector<OutputStream> streams;
	std::vector<std::unique_ptr<BasePipe> > heap_allocated;

	std::thread head;
	std::atomic<bool> is_running{false};

	struct OutputStream {
		friend class VisionServer;
	public:
		inline static const std::shared_ptr<nt::NetworkTable>
			streams_table = base_table->GetSubTable("Streams");

		inline OutputStream(std::string_view n) :
			OutputStream(frc::CameraServer::AddServer(n)) {}
		inline OutputStream(std::string_view n, int p) :
			OutputStream(frc::CameraServer::AddServer(n, p)) {}
		OutputStream(cs::MjpegServer&& s);	// setup ntable values
		~OutputStream() = default;	// remove ntable values

	private:
		cs::MjpegServer server;
		const std::shared_ptr<nt::NetworkTable> table;

	};


};



template<class derived>	// CRTP for instance counting -> all derived classes should inherit a passthrough template if they are expected to be extended as well
class VPipeline : public Instanced<VPipeline<derived> >, public VisionServer::BasePipe {
	typedef struct VPipeline<derived>	This_t;
protected:
	VPipeline() = delete;
	VPipeline(const VPipeline&) = delete;
	inline VPipeline(const char* name) : 
		Instanced<This_t>(), BasePipe(name + std::to_string(this->instance)) {}
	inline VPipeline(const std::string& name) :
		Instanced<This_t>(), BasePipe(name + std::to_string(this->instance)) {}
	inline VPipeline(std::string&& name) :
		Instanced<This_t>(), BasePipe(name + std::to_string(this->instance)) {}
	inline virtual ~VPipeline() = default;


	virtual void process(cv::Mat& io_frame) override;


};

template<class... pipelines_t>
class SequentialPipeline : public VPipeline<SequentialPipeline<pipelines_t...> > {
	typedef struct SequentialPipeline<pipelines_t...>		This_t;
public:
	inline SequentialPipeline() :
		VPipeline<This_t>("SequentialPipeline<" + Construct(this->heap_ptrs) + "> ")
	{
		for(size_t i = 0; i < this->heap_ptrs.size(); i++) {
			this->pipelines.push_back(this->heap_ptrs.at(i).get());
		}
	}

	void addPipeline(VisionServer::BasePipe*);
	void addPipelines(std::vector<VisionServer::BasePipe*>&&);
	void addPipelines(std::initializer_list<VisionServer::BasePipe*>);


	virtual void process(cv::Mat& io_frame) override;

protected:
	static std::string Construct(std::vector<std::unique_ptr<VisionServer::BasePipe> >&);

	std::vector<VisionServer::BasePipe*> pipelines;
	std::vector<std::unique_ptr<VisionServer::BasePipe> > heap_ptrs;


};
typedef SequentialPipeline<>	SeqPipeline;


}	// namespace vs2


#include "visionserver2.inc"