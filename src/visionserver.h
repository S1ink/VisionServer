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

#include "extras/resources.h"
#include "visioncamera.h"

inline cv::Size getResolution(cs::VideoMode vm) {
    return cv::Size(vm.height, vm.width);
}

class VisionServer;

class PipelineBase {
public:
    PipelineBase(VisionServer* server); //const?
    virtual ~PipelineBase() = default;

    virtual void process(cv::Mat& frame, cs::CvSource& output);

protected:
    VisionServer* env;

    std::vector<VisionCamera>* getEnvCameras();
    cs::CvSink getEnvSource();
    cs::CvSource getEnvStream();
    CHRONO::high_resolution_clock::time_point getEnvStart();

};

class VisionServer {
    friend class PipelineBase;
public:
    VisionServer(std::vector<VisionCamera>& cameras);

    size_t validIndexes();
    bool setCamera(size_t idx);
    cv::Size getCurrentResolution();

    bool stopVision();

    template<class pipeline_t = PipelineBase>
    void runVision() {
        static_assert(std::is_base_of<PipelineBase, pipeline_t>::value, "Template argument (pipeline_t) must inherit from PipelineBase");
        pipeline_t pipeline(this);
        cv::Mat frame(this->getCurrentResolution(), CV_8UC3);   // frame may need to be resized if a camera is switched, but this has not been tested
        this->start = CHRONO::high_resolution_clock::now();
        while(this->runloop) {   // add condition with external link
            this->source.GrabFrame(frame);
            pipeline.process(frame, this->stream);
        }
        this->runloop = true;
    }
    template<class pipeline_t = PipelineBase>
    bool runVisionThread() {
        static_assert(std::is_base_of<PipelineBase, pipeline_t>::value, "Template argument (pipeline_t) must inherit from PipelineBase");
        if(!this->launched.joinable()) {
            this->launched = std::move(std::thread(VisionServer::visionWorker<pipeline_t>, this));
            return true;
        }
        return false;
    }

protected:
    template<typename pipeline_t>
    static void visionWorker(VisionServer* server) {
        pipeline_t pipeline(server);
        cv::Mat frame(server->getCurrentResolution(), CV_8UC3);
        server->start = CHRONO::high_resolution_clock::now();
        while(server->runloop) {
            server->source.GrabFrame(frame);
            pipeline.process(frame, server->stream);
        }
        server->runloop = true;
    }

    std::vector<VisionCamera>* cameras;
    cs::CvSink source;
    cs::CvSource stream;
    
    CHRONO::high_resolution_clock::time_point start;

    std::atomic_bool runloop{true};
    std::thread launched;

    std::shared_ptr<nt::NetworkTable> table{nt::NetworkTableInstance::GetDefault().GetTable("Vision Server")};

};