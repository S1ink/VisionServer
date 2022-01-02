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

/**
 * A pipeline interface that must be implemented by each pipeline to be used by VisionServer
*/
class VPipeline {       // Vision Pipeline base interface
public:
    /**
     * Construct a pipeline
     * @param server The surrounding VisionServer instance - passed in when pipelines are created within a processing instance (ex. VisionServer::runVision)
    */
    VPipeline(VisionServer& server);
    /**
     * Construct a pipeline with a name
     * @param server The surrounding VisionServer instance - passed in when pipelines are created within a processing instance (ex. VisionServer::runVision)
     * @param ntable The name of the pipeline and networktable that will be created
    */
    VPipeline(VisionServer& server, const char* ntable);
    /**
     * Construct a pipeline with a name
     * @param server The surrounding VisionServer instance - passed in when pipelines are created within a processing instance (ex. VisionServer::runVision)
     * @param ntable The name of the pipeline and networktable that will be created
    */
    VPipeline(VisionServer& server, const std::string& ntable);
    /**
     * Oftentimes pipelines have fairly large framebuffers (cv::Mat) and lots of other members, so we don't want them to be copied
    */
    VPipeline(const VPipeline&) = delete;
    /**
     * Move constructor
    */
    VPipeline(VPipeline&&);
    virtual ~VPipeline();

    /**
     * Oftentimes pipelines have fairly large framebuffers (cv::Mat) and lots of other members, so we don't want them to be copy assigned
    */
    VPipeline& operator=(const VPipeline&) = delete;
    //VPipeline& operator=(VPipeline&&);

    /**
     * Get the pipeline name
     * @return The name of the pipeline
    */
    const std::string& getName() const;
    /**
     * Get the underlying networktable for the pipeline
     * @return A const shared pointer to the pipeline's networktable
    */
    const std::shared_ptr<nt::NetworkTable> getTable() const;
    /**
     * Get the VisionServer instance that the pipeline was created with
     * @return A const pointer to the VisionServer isntance
    */
    const VisionServer* getEnv() const;

    /**
     * Processing function to be overloaded by each pipeline
     * @param io_frame The frame that is recieved from a camera and is sent to the stream after processing finishes
     * @param mode Indicates modes (settings from VisionServer networktable) to be implemented during processing - currently not used
    */
    virtual void process(cv::Mat& io_frame, int8_t mode = 0) = 0;

    /**
     * Get the current camera's matrix array from the VisionServer instance (to be used by classes that extend this)
     * @return The camera matrix - size is 3x3 (float)
    */
    const cv::Mat_<float>& getCameraMatrix() const;
    /**
     * Get the current camera's distortion coefficients from the VisionServer instance (to be used by classes that extend this)
     * @return The distortion coefficients - size if 5x1 (float)
    */
    const cv::Mat_<float>& getCameraDistortion() const;
    /**
     * Update the current target by passing it's name
     * @param target The target's name - can be gotten by calling 'getName()' on a target
    */
    void updateTarget(const std::string& target);

protected:
    std::string name;
    const std::shared_ptr<nt::NetworkTable> table;
    VisionServer* env;

};
/**
 * A default pipeline implentation that does no processing but still overloads process(), and thus can be used in a processing instance
*/
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


/**
 * Manages cameras, base networktables communication, statistics, target notifications, and pipeline instances
*/
class VisionServer {    // make singleton?
    friend class VPipeline;
public:
    /**
     * Parse a json to construct cameras
     * @param file Path to the json, default it "/boot/frc.json"
    */
    VisionServer(const char* file = _default);
    /**
     * Uses preexisting cameras
     * @param cameras A vector of VisionCameras that will be used for processing
    */
    VisionServer(std::vector<VisionCamera>&& cameras);
    VisionServer(const VisionServer&) = delete;
    VisionServer(VisionServer&&) = delete;
    /**
     * Deletes all networktable entries created by the instance
    */
    ~VisionServer();

    /**
     * Gets the number of cameras avaiable
     * @return The number of avaiable camera indexes, starting at 0
    */
    size_t validIndexes() const;
    /**
     * Set a specific camera index as a source for the output stream (and processing)
     * @param idx The index of the camera, starting at 0 and <= validIndexes()
    */
    bool setCamera(size_t idx);
    /**
     * Get the frame size of the current camera
     * @return The dimensions of the output frame in a cv::Size object
    */
    cv::Size getCurrentResolution() const;
    /**
     * Set the default compression of the Mjpeg stream output
     * @param quality compression quality, ranging from 0-100 (100 being the highest quality), and -1 for auto
    */
    void setCompression(int8_t quality);

    /**
     * Get a const reference to the internally stored camera vector
     * @return Returns a const reference to the internal vector of VisionCameras
    */
    const std::vector<VisionCamera>& getCameras();

    /**
     * Get the camera matrix for the currently selected camera
     * @return A cv::Mat(of floats) array representing the current camera's matrix - the size is 3x3
    */
    const cv::Mat_<float>& getCameraMatrix() const;
    /**
     * Get the distortion coefficients for the currently selected camera
     * @return A cv::Mat(of floats) array representing the current camera's distortion coefficients - the size is 5x1
    */
    const cv::Mat_<float>& getDistortion() const;

    /**
     * Joins any processing threads that are currently running
     * @return Returns if there was a thread running that got stopped
    */
    bool stopVision();

    /**
     * Run a streaming instance with no processing pipelines
     * @param quality The default streaming quality
    */
    inline void runVision(int8_t quality = 50) { VisionServer::visionWorker(*this, quality); }
    /**
     * Runs a processing instance with 1 pipeline
     * @param pipeline_t The typename of the pipeline class that will be used
     * @param quality The default streaming quality
    */
    template<class pipeline_t>
    inline void runVision(int8_t quality = 50) { VisionServer::visionWorker<pipeline_t>(*this, quality); }
    /**
     * Runs a processing instance with 2 pipelines (switchable with networktables)
     * @param pipeline_t1 The typename of the first pipeline class that will be used
     * @param pipeline_t2 The typename of the second pipeline class that will be used
     * @param quality The default streaming quality
    */
    template<class pipeline_t1, class pipeline_t2>
    inline void runVision(int8_t quality = 50) { VisionServer::visionWorker<pipeline_t1, pipeline_t2>(*this, quality); }
    /**
     * Runs a processing instance with 3 pipelines (switchable with networktables)
     * @param pipeline_t1 The typename of the first pipeline class that will be used
     * @param pipeline_t2 The typename of the second pipeline class that will be used
     * @param pipeline_t3 The typename of the third pipeline class that will be used
     * @param quality The default streaming quality
    */
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