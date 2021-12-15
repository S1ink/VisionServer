#pragma once

#include <networktables/NetworkTableInstance.h>
#include <networktables/EntryListenerFlags.h>
#include <networktables/NetworkTableEntry.h>
#include <networktables/NetworkTable.h>
#include <cameraserver/CameraServer.h>

#include <opencv2/opencv.hpp>

#include <type_traits>
#include <thread>
#include <chrono>
#include <atomic>
#include <tuple>

#include "extras/resources.h"
#include "extras/stats.h"
#include "visioncamera.h"

class VisionServer;

// class PipelineBase {     // legacy pipeline base class
// public:
//     explicit PipelineBase(VisionServer* server);
//     PipelineBase(const PipelineBase& other) = delete;
//     virtual ~PipelineBase() = default;
//     PipelineBase& operator=(const PipelineBase& other) = delete;

//     virtual void process(cv::Mat& io_frame, bool show_thresh = false);
//     //virtual std::shared_ptr<nt::NetworkTable> getTable();

// protected:
//     VisionServer* env;

//     CHRONO::high_resolution_clock::time_point getEnvStart();
//     const cv::Mat_<float>& getCameraMatrix();
//     const cv::Mat_<float>& getDistortion();
//     void updateMatrices(const cv::Mat_<float>& tvec, const cv::Mat_<float>& rvec);
//     void updateMatrices(const cv::Mat_<float>& tvec);

// };

class VPipeline {       // Vision Pipeline base interface
public:
    //VPipeline() = default;
    explicit VPipeline(VisionServer& server);
    VPipeline(const VPipeline&) = delete;
    virtual ~VPipeline() = default;
    VPipeline& operator=(const VPipeline&) = delete;

    virtual void process(cv::Mat& io_frame, bool debug = false) = 0;
    //void setEnv(const VisionServer* server);

    const cv::Mat_<float>& getCameraMatrix() const;
    const cv::Mat_<float>& getCameraDistortion() const;

    void updateMatrices(const cv::Mat_<float>& tvec, const cv::Mat_<float>& rvec);
    void updateMatrices(const cv::Mat_<float>& tvec);

    const VisionServer* getEnv() const;

private:
    VisionServer* env;
    
};
class DefaultPipeline : VPipeline {     // default pipeline (video passthrough)
    using VPipeline::VPipeline;
public:
    void process(cv::Mat& io_frame, bool debug = false) override {}

};



class VisionServer {
    friend class PipelineBase;
    friend class VPipeline;
public:
    VisionServer(std::vector<VisionCamera>& cameras);
    ~VisionServer() {}    // implement this to delete all networktable entries created by the server

    size_t validIndexes() const;
    bool setCamera(size_t idx);
    cv::Size getCurrentResolution() const;
    void setCompression(int8_t quality);

    const cv::Mat_<float>& getCameraMatrix() const;
    const cv::Mat_<float>& getDistortion() const;

    bool stopVision();

    template<class pipeline_t = DefaultPipeline>
    void runVision(int8_t quality = 50);
    template<class pipeline_t1, class pipeline_t2>
    void runVision(int8_t quality = 50);
    template<class pipeline_t1, class pipeline_t2, class pipeline_t3>
    void runVision(int8_t quality = 50);

    template<class pipeline_t = DefaultPipeline>
    bool runVisionThread(int8_t quality = 50);
    template<class pipeline_t1, class pipeline_t2>
    bool runVisionThread(int8_t quality = 50);
    template<class pipeline_t1, class pipeline_t2, class pipeline_t3>
    bool runVisionThread(int8_t quality = 50);

    //void runVisionTest(int8_t quality = 50);

protected:
    template<class pipeline_t>
    static void visionWorker(VisionServer& server, int8_t quality = 50);
    template<class pipeline_t1, class pipeline_t2>
    static void visionWorker(VisionServer& server, int8_t quality = 50);
    template<class pipeline_t1, class pipeline_t2, class pipeline_t3>
    static void visionWorker(VisionServer& server, int8_t quality = 50);

    void putStats(cv::Mat& io_frame);

    void updateMatrices(const cv::Mat_<float>& tvec, const cv::Mat_<float>& rvec);
    void updateMatrices(const cv::Mat_<float>& tvec);

    std::vector<VisionCamera>* cameras;
    cs::CvSink source;
    cs::CvSource output;
    cs::MjpegServer stream;

    cv::Mat_<float> camera_matrix{cv::Mat_<float>(3, 3)}, distortion{cv::Mat_<float>(1, 5)};

    std::atomic_bool runloop{true};
    std::thread launched;

    std::shared_ptr<nt::NetworkTable> table = {nt::NetworkTableInstance::GetDefault().GetTable("Vision Server")};

    uint64_t total_frames{0}, sec1_frames{0};
	double total_time{0.f}, frame_time{0.f}, loop_time{0.f}, sec1_time{0.f}, fps_1s{0.f}, fps{0.f};
	CHRONO::high_resolution_clock::time_point start, beg, end, last;

};

#include "visionserver.inc"