#pragma once

#include <networktables/NetworkTableInstance.h>
#include <networktables/EntryListenerFlags.h>
#include <networktables/NetworkTableEntry.h>
#include <networktables/NetworkTable.h>
#include <cameraserver/CameraServer.h>

#include <opencv2/core/types.hpp>

#include <type_traits>
#include <thread>
#include <chrono>
#include <atomic>
#include <tuple>

#include "extras/resources.h"
#include "visioncamera.h"

class VisionServer;

class PipelineBase {
public:
    PipelineBase(const VisionServer* server);
    virtual ~PipelineBase() = default;
    virtual void process(cv::Mat& frame, cs::CvSource& output, bool show_thresh = false);   // make this return the frametime (double)

protected:
    const VisionServer* env;
    CHRONO::high_resolution_clock::time_point getEnvStart();
};



class VisionServer {
    friend class PipelineBase;
public:
    VisionServer(std::vector<VisionCamera>& cameras);

    size_t validIndexes() const;
    bool setCamera(size_t idx);
    cv::Size getCurrentResolution() const;

    bool stopVision();

    template<class pipeline_t = PipelineBase>
    void runVision();
    template<class pipeline_t1, class pipeline_t2>
    void runVision();
    template<class pipeline_t1, class pipeline_t2, class pipeline_t3>
    void runVision();
    template<class pipeline_t = PipelineBase>
    bool runVisionThread();

protected:
    template<typename pipeline_t>
    static void visionWorker(VisionServer* server);

    std::vector<VisionCamera>* cameras;
    cs::CvSink source;
    cs::CvSource stream;
    
    CHRONO::high_resolution_clock::time_point start;

    std::atomic_bool runloop{true};
    std::thread launched;

    std::shared_ptr<nt::NetworkTable> table = {nt::NetworkTableInstance::GetDefault().GetTable("Vision Server")};

};

#include "visionserver.inc"