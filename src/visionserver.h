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

    template<class pipeline_t = PipelineBase>
    void runVision() {
        static_assert(std::is_base_of<PipelineBase, pipeline_t>::value, "Template argument must inherit from PipelineBase");
        pipeline_t pipeline(this);
        cv::Mat frame(this->getCurrentResolution(), CV_8UC3);   // frame may need to be resized if a camera is switched, but this has not been tested
        this->start = CHRONO::high_resolution_clock::now();
        for(;;) {   // add condition with external link
            this->source.GrabFrame(frame);
            pipeline.process(frame, this->stream);
        }
    }
    // template<class pipeline_t>
    // void runMultiVision(std::vector<PipelineBase&> pipelines) {

    // }
    //static std::thread runVisionThread(VisionServer* server, PipelineBase& pipeline);
    //static std::thread runMultiVisionThread(VisionServer* server, std::vector<PipelineBase&> pipelines);

protected:
    //static void visionWorker(VisionServer* server, PipelineBase& pipeline);
    //static void multiVisionWorker(VisionServer* server, std::vector<PipelineBase&> pipelines);

    std::vector<VisionCamera>* cameras;

    cs::CvSink source;
    cs::CvSource stream;
    
    CHRONO::high_resolution_clock::time_point start;

    std::shared_ptr<nt::NetworkTable> table{nt::NetworkTableInstance::GetDefault().GetTable("Vision Server")};

};