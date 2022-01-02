#pragma once

#include <networktables/NetworkTableInstance.h>
#include <networktables/NetworkTable.h>
#include <cameraserver/CameraServer.h>

#include <opencv2/opencv.hpp>

#include <thread>
#include <chrono>
#include <atomic>
#include <string>

#include "tools/src/resources.h"
#include "visioncamera.h"
#include "vision.h"

class VisionServer;

class VPipeline {       // Vision Pipeline base interface
public:
    VPipeline(VisionServer& server);
    VPipeline(VisionServer& server, const char* ntable);
    VPipeline(VisionServer& server, const std::string& ntable);
    VPipeline(const VPipeline&) = delete;
    VPipeline(VPipeline&&);
    virtual ~VPipeline();

    VPipeline& operator=(const VPipeline&) = delete;
    //VPipeline& operator=(VPipeline&&);

    const std::string& getName() const;
    const std::shared_ptr<nt::NetworkTable> getTable() const;
    const VisionServer* getEnv() const;

    virtual void process(cv::Mat& io_frame, int8_t mode = 0) = 0;

    const cv::Mat_<float>& getCameraMatrix() const;
    const cv::Mat_<float>& getCameraDistortion() const;
    void updateTarget(const std::string& target);

protected:
    std::string name;
    const std::shared_ptr<nt::NetworkTable> table;
    VisionServer* env;

};
class DefaultPipeline : VPipeline {     // default pipeline (video passthrough)
public:
    //using VPipeline::VPipeline;
    explicit DefaultPipeline(VisionServer&);
    DefaultPipeline(const DefaultPipeline&) = delete;
    //DefaultPipeline(DefaultPipeline&&);
    ~DefaultPipeline() = default;
    DefaultPipeline& operator=(const DefaultPipeline&) = delete;
    //DefaultPipeline& operator=(DefaultPipeline&&);

    void process(cv::Mat& io_frame, int8_t mode = 0) override {}

};



class VisionServer {    // make singleton?
    friend class VPipeline;
public:
    VisionServer(const char* file = _default);
    VisionServer(std::vector<VisionCamera>&& cameras);
    VisionServer(const VisionServer&) = delete;
    VisionServer(VisionServer&&) = delete;
    ~VisionServer();

    size_t validIndexes() const;
    bool setCamera(size_t idx);
    cv::Size getCurrentResolution() const;
    void setCompression(int8_t quality);

    const std::vector<VisionCamera>& getCameras();

    const cv::Mat_<float>& getCameraMatrix() const;
    const cv::Mat_<float>& getDistortion() const;

    bool stopVision();

    inline void runVision(int8_t quality = 50) { VisionServer::visionWorker(*this, quality); }
    template<class pipeline_t>
    inline void runVision(int8_t quality = 50) { VisionServer::visionWorker<pipeline_t>(*this, quality); }
    template<class pipeline_t1, class pipeline_t2>
    inline void runVision(int8_t quality = 50) { VisionServer::visionWorker<pipeline_t1, pipeline_t2>(*this, quality); }
    template<class pipeline_t1, class pipeline_t2, class pipeline_t3>
    inline void runVision(int8_t quality = 50) { VisionServer::visionWorker<pipeline_t1, pipeline_t2, pipeline_t3>(*this, quality); }

    // Nope, too confusing
    // template<typename... pipelines_t>
    // void runVision(int8_t quality = 50);

    bool runVisionThread(int8_t quality = 50);
    template<class pipeline_t>
    bool runVisionThread(int8_t quality = 50);
    template<class pipeline_t1, class pipeline_t2>
    bool runVisionThread(int8_t quality = 50);
    template<class pipeline_t1, class pipeline_t2, class pipeline_t3>
    bool runVisionThread(int8_t quality = 50);

    const std::shared_ptr<nt::NetworkTable> vision{nt::NetworkTableInstance::GetDefault().GetTable("Vision Server")};

protected:
    static void visionWorker(VisionServer& server, int8_t quality);
    template<class pipeline_t>
    static void visionWorker(VisionServer& server, int8_t quality);
    template<class pipeline_t1, class pipeline_t2>
    static void visionWorker(VisionServer& server, int8_t quality);
    template<class pipeline_t1, class pipeline_t2, class pipeline_t3>
    static void visionWorker(VisionServer& server, int8_t quality);

    // nope
    // template<typename... pipelines>
    // static void visionWorker(VisionServer& server, int8_t quality);

    bool updateFromConfig(const char* file = _default);

    void putStats(cv::Mat& io_frame);
    void updateStats();

    void updateTarget(const std::string& target);

    std::vector<VisionCamera> cameras;
    cs::CvSink source;
    cs::CvSource output;
    cs::MjpegServer stream;

    cv::Mat_<float> camera_matrix{cv::Mat_<float>(3, 3)}, distortion{cv::Mat_<float>(1, 5)};

    std::atomic_bool runloop{true};
    std::thread launched;

    // move to methods?
    uint64_t total_frames{0}, sec1_frames{0};
	double total_time{0.f}, frame_time{0.f}, loop_time{0.f}, sec1_time{0.f}, fps_1s{0.f}, fps{0.f};
	CHRONO::high_resolution_clock::time_point start, beg, end, last;

private:
    class TargetInfo {
    public:
        TargetInfo(const std::shared_ptr<nt::NetworkTable> table);
        TargetInfo(const TargetInfo&) = delete;
        TargetInfo(TargetInfo&&) = default;
        ~TargetInfo() = default;

        void setTarget(const std::string& target);
        bool update(double ltime);

    private:
        const std::shared_ptr<nt::NetworkTable> ttable;
        CHRONO::high_resolution_clock::time_point last; 

    } active_target{vision};

};

#include "visionserver.inc"