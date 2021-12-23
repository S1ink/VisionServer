#pragma once

#include <networktables/NetworkTableInstance.h>
#include <networktables/NetworkTable.h>
#include <cameraserver/CameraServer.h>
#include <wpi/Twine.h>

#include <opencv2/opencv.hpp>

#include <thread>
#include <chrono>
#include <atomic>

#include "extras/resources.h"
#include "visioncamera.h"

class VisionServer;

enum class VFlag {
    NONE = 0b000,       // signifies video passthrough, no markup
    DEBUG = 0b001,      // debug (info/stats)
    THRESH = 0b010,     // show threshold
    DEMO = 0b100,       // demo (markup)
};

class VPipeline {       // Vision Pipeline base interface
public:
    //VPipeline() = default;
    explicit VPipeline(VisionServer& server);
    explicit VPipeline(VisionServer& server, const char* ntable);
    explicit VPipeline(VisionServer& server, const wpi::Twine& ntable);
    VPipeline(const VPipeline&) = delete;
    virtual ~VPipeline() = default;
    VPipeline& operator=(const VPipeline&) = delete;

    virtual void process(cv::Mat& io_frame, uint8_t mode = (uint8_t)VFlag::NONE) = 0;

    const cv::Mat_<float>& getCameraMatrix() const;
    const cv::Mat_<float>& getCameraDistortion() const;

    void updateTarget(const std::string& target);

    // void updateMatrices(const cv::Mat_<float>& tvec, const cv::Mat_<float>& rvec);
    // void updateMatrices(const cv::Mat_<float>& tvec);

    const std::shared_ptr<nt::NetworkTable> getTable() const;
    const VisionServer* getEnv() const;

protected:
    const std::shared_ptr<nt::NetworkTable> table;

private:
    VisionServer* env;
    
};
class DefaultPipeline : VPipeline {     // default pipeline (video passthrough)
public:
    //using VPipeline::VPipeline;
    explicit DefaultPipeline(VisionServer&);
    DefaultPipeline(const DefaultPipeline&) = delete;
    DefaultPipeline& operator=(const DefaultPipeline&) = delete;

    void process(cv::Mat& io_frame, uint8_t mode = (uint8_t)VFlag::NONE) override {}

};



// class Position {
// public:
//     static Position& Get();

//     void setPos(float x, float y, float z);
//     void setDistance(double d);
//     void setThetaUD(double deg);
//     void setThetaLR(double deg);
    
//     void setAll(const cv::Mat_<float>& tvec);

// private:
//     Position() = default;
//     Position(const Position& other) = delete;

//     const std::shared_ptr<nt::NetworkTable> pos_table{nt::NetworkTableInstance::GetDefault().GetTable("robot/position")};

// };



class VisionServer {    // make singleton?
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
    static void visionWorker(VisionServer& server, int8_t quality);
    template<class pipeline_t1, class pipeline_t2>
    static void visionWorker(VisionServer& server, int8_t quality);
    template<class pipeline_t1, class pipeline_t2, class pipeline_t3>
    static void visionWorker(VisionServer& server, int8_t quality);

    void putStats(cv::Mat& io_frame);
    void updateStats();

    void setupNtVflag();
    uint8_t getNtVflag();
    void deleteNtVflag();

    void updateTarget(const std::string& target);

    // void updateMatrices(const cv::Mat_<float>& tvec, const cv::Mat_<float>& rvec);
    // void updateMatrices(const cv::Mat_<float>& tvec);

    std::vector<VisionCamera>* cameras;
    cs::CvSink source;
    cs::CvSource output;
    cs::MjpegServer stream;

    cv::Mat_<float> camera_matrix{cv::Mat_<float>(3, 3)}, distortion{cv::Mat_<float>(1, 5)};

    std::atomic_bool runloop{true};
    std::thread launched;

    const std::shared_ptr<nt::NetworkTable> table{nt::NetworkTableInstance::GetDefault().GetTable("Vision Server")};

    // move to methods as these only need to be local -> can then run multiple threads at once off of the same server
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

    } active_target{table};

};

#include "visionserver.inc"