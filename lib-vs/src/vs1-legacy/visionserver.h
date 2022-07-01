#pragma once

#include <networktables/NetworkTableInstance.h>
#include <networktables/NetworkTable.h>
#include <cameraserver/CameraServer.h>

#include <opencv2/opencv.hpp>

#include <thread>
#include <chrono>
#include <atomic>
#include <string>
#include <memory>

#include "cpp-tools/src/resources.h"
#include "visioncamera.h"
#include "vision.h"


namespace vs1 {

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
    inline const std::string& getName() const { return this->name; }
    /**
     * Get the underlying networktable for the pipeline
     * @return A const shared pointer to the pipeline's networktable
    */
    inline const std::shared_ptr<nt::NetworkTable> getTable() const { return this->table; }
    /**
     * Get the VisionServer instance that the pipeline was created with
     * @return A const pointer to the VisionServer isntance
    */
    inline const VisionServer* getEnv() const { return this->env; }

    /**
     * Processing function to be overloaded by each pipeline
     * @param io_frame The frame that is recieved from a camera and is sent to the stream after processing finishes
     * @param mode Indicates modes (settings from VisionServer networktable) to be implemented during processing - currently not used
    */
    virtual void process(cv::Mat& io_frame) = 0;

    /**
     * Update the current target by passing it's name
     * @param target The target's name - can be gotten by calling 'getName()' on a target
    */
    void updateTarget(const std::string& target);
    /**
     * Get access to the current camera source
     * @return The camera as a VisionCamera
    */
    const VisionCamera& getCurrentCamera() const;
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

protected:
    /**The pipeline's name*/
    std::string name;
    /**A networktable for the pipeline*/
    const std::shared_ptr<nt::NetworkTable> table;
    /**A pointer to the surrounding VisionServer instance*/
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

    void process(cv::Mat& io_frame) override {}


};


/**
 * Manages cameras, base networktables communication, statistics, target notifications, and pipeline instances
*/
class VisionServer {    // make singleton?
    friend class VPipeline;
public:
    /**
     * Parse a json to construct cameras
     * @param file Path to the json, default is "/boot/frc.json"
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
     * Get the number of cameras avaiable
     * @return The number of avaiable camera indexes, starting at 0
    */
    inline size_t validIndexes() const { return this->cameras.size(); }
    /**
     * Get the current camera index
     * @return The index of the current camera
    */
    inline size_t getCurrentIndex() const { return this->vision->GetEntry("Camera Index").GetDouble(0); }
    /**
     * Set a specific camera index as a source for the output stream (and processing)
     * @param idx The index of the camera, starting at 0 and <= validIndexes()
    */
    bool setCamera(size_t idx);
    /**
     * Get the frame size of the current camera
     * @return The dimensions of the output frame in a cv::Size object
    */
    inline cv::Size getCurrentResolution() const { return this->selected ? this->selected->getResolution() : cv::Size(); }
    /**
     * Set the default compression of the Mjpeg stream output
     * @param quality compression quality, ranging from 0-100 (100 being the highest quality), and -1 for auto
    */
    void setCompression(int8_t quality);

    /**
     * Get a const reference to the internally stored camera vector
     * @return Returns a const reference to the internal vector of VisionCameras
    */
    inline const std::vector<VisionCamera>& getCameras() const { return this->cameras; }

    /**
     * Get a const reference to the currently selected camera's object. Use this to aquire frames and calibration matrices.
     * @return a const reference to the currently selected VisionCamera object
    */
    inline const VisionCamera& getCurrentCamera() const { return *this->selected; }

    /**
     * Joins any processing threads that are currently running
     * @return Returns true if there was a thread running that was joined
    */
    bool stopVision();

    /**
     * Run a streaming instance with no processing pipelines. Exactly the same as runVision<>() but does not push 
	 * networktable values for changing pipeline indexes (and is one if-statement more efficent per frame).
     * @param quality The default streaming quality
    */
    inline void runVision(int8_t quality = 50) { VisionServer::visionWorker(*this, quality); }
    /**
     * Runs a processing instance with 1 pipeline. The pipeline instance is stack allocated rather than 
	 * heap allocated like with runVision<pipelines...>()
     * @param pipeline_t The typename of the pipeline class that will be used
     * @param quality The default streaming quality
    */
    template<class pipeline_t>
    inline void runVision_S(int8_t quality = 50) { VisionServer::visionWorker_S<pipeline_t>(*this, quality); }
    /**
     * Runs a processing instance with 2 pipelines (switchable via networktables). Pipeline instances are 
	 * stack allocated rather than heap allocated like with runVision<pipelines...>().
     * @param pipeline_t1 The typename of the first pipeline class that will be used
     * @param pipeline_t2 The typename of the second pipeline class that will be used
     * @param quality The default streaming quality
    */
    template<class pipeline_t1, class pipeline_t2>
    inline void runVision_S(int8_t quality = 50) { VisionServer::visionWorker_S<pipeline_t1, pipeline_t2>(*this, quality); }
    /**
     * Runs a processing instance with 3 pipelines (switchable via networktables). Pipeline instances are 
	 * stack allocated rather than heap allocated like with runVision<pipelines...>().
     * @param pipeline_t1 The typename of the first pipeline class that will be used
     * @param pipeline_t2 The typename of the second pipeline class that will be used
     * @param pipeline_t3 The typename of the third pipeline class that will be used
     * @param quality The default streaming quality
    */
    template<class pipeline_t1, class pipeline_t2, class pipeline_t3>
    inline void runVision_S(int8_t quality = 50) { VisionServer::visionWorker_S<pipeline_t1, pipeline_t2, pipeline_t3>(*this, quality); }
    /**
     * Runs a processing instace with however many pipelines are supplied (although heap-allocation is used). Pipelines are switchable via networktables.
     * @param pipelines The pipeline typenames, separated by commas
     * @param quality The default streaming quality
    */
   	template<class... pipelines>
	inline void runVision(int8_t quality = 50) { VisionServer::visionWorker<pipelines...>(*this, quality); }


    /**
     * Runs a streaming thread with no processing pipelines
     * @param quality The default streaming quality
    */
    bool runVisionThread(int8_t quality = 50);
    /**
     * Runs a processing thread with 1 stack allocated pipeline.
     * @param pipeline_t The typename of the pipeline class that will be used
     * @param quality The default streaming quality
    */
    template<class pipeline_t>
    bool runVisionThread_S(int8_t quality = 50);
    /**
     * Runs a processing thread with 2 stack allocated pipelines (switchable via networktables)
     * @param pipeline_t1 The typename of the first pipeline class that will be used
     * @param pipeline_t2 The typename of the second pipeline class that will be used
     * @param quality The default streaming quality
    */
    template<class pipeline_t1, class pipeline_t2>
    bool runVisionThread_S(int8_t quality = 50);
    /**
     * Runs a processing thread with 3 stack allocated pipelines (switchable via networktables)
     * @param pipeline_t1 The typename of the first pipeline class that will be used
     * @param pipeline_t2 The typename of the second pipeline class that will be used
     * @param pipeline_t3 The typename of the third pipeline class that will be used
     * @param quality The default streaming quality
    */
    template<class pipeline_t1, class pipeline_t2, class pipeline_t3>
    bool runVisionThread_S(int8_t quality = 50);
	/**
     * Runs a processing thread with however many pipelines are supplied (although heap-allocation is used). Pipelines are switchable via networktables.
     * @param pipelines The pipeline typenames, separated by commas
     * @param quality The default streaming quality
    */
	template<class... pipelines>
	bool runVisionThread(int8_t quality = 50);

    /**
     * The root networktable for all VisionServer entries
    */
    const std::shared_ptr<nt::NetworkTable> vision{nt::NetworkTableInstance::GetDefault().GetTable("Vision Server")};

protected:
    /**The underlying worker function called by both runVision() and runVisionThread()*/
    static void visionWorker(VisionServer& server, int8_t quality);
    /**The underlying worker function called by both runVision_S<pipeline_t>() and runVisionThread_S<pipeline_t>()*/
    template<class pipeline_t>
    static void visionWorker_S(VisionServer& server, int8_t quality);
    /**The underlying worker function called by both runVision_S<pipeline_t1, pipeline_t2>() and runVisionThread_S<pipeline_t1, pipeline_t2>()*/
    template<class pipeline_t1, class pipeline_t2>
    static void visionWorker_S(VisionServer& server, int8_t quality);
    /**The underlying worker function called by both runVision_S<pipeline_t1, pipeline_t2, pipeline_t3>() and runVisionThread_S<pipeline_t1, pipeline_t2, pipeline_t3>()*/
    template<class pipeline_t1, class pipeline_t2, class pipeline_t3>
    static void visionWorker_S(VisionServer& server, int8_t quality);
    /**The underlying worker function called by both runVison<pipelines...>() and runVisionThread<pipelines...>()*/
    template<class... pipelines>
    static void visionWorker(VisionServer& server, int8_t quality);

    /**A tool for expanding pipeline variadic template params and emplacing instances into the supplied vector*/
    template<class pipeline = void, class... pipelines>
    static void pipeExpansion(std::vector<std::unique_ptr<VPipeline> >& pipes, VisionServer& server);

    /**
     * Called from VisionServer(const char*) to parse the json and create cameras - can be called again to update the cameras
     * @param file The path to the source json - default is "/boot/frc.json"
    */
    bool updateFromConfig(const char* file = _default);

    /**
     * Puts current fps and frametime statistics in the upper-left corner of a frame
     * @param io_frame The frame to be modified
    */
    void putStats(cv::Mat& io_frame);
    /**
     * Updates networtables with current statistics - called within each processing instance at each frame
    */
    void updateStats();

    /**
     * Gives pipelines access to updating the current target
     * @param target The name of the target that is actively being tracked
    */
    inline void updateTarget(const std::string& target) { this->active_target.setTarget(target); }

   	inline cs::CvSource& getOutput() { return this->output; }

    std::vector<VisionCamera> cameras;
    const VisionCamera* selected;
    cs::CvSource output;
    cs::MjpegServer stream;

    std::atomic_bool runloop{true};
    std::thread launched;

    // move to methods?
    uint64_t total_frames{0}, sec1_frames{0};
	double total_time{0.f}, frame_time{0.f}, loop_time{0.f}, sec1_time{0.f}, fps_1s{0.f}, fps{0.f};
	CHRONO::high_resolution_clock::time_point start, beg, end, last;

private:
    /**
     * Manages pushing data for the active target to networktables and reseting after enough time with no target detection. 
     * This is necessary so that a robot never gets stuck reusing old targeting data when a target goes out of view
    */
    class TargetInfo {
    public:
        /**
         * @param table The root networktable that targeting info is contained in
        */
        TargetInfo(const std::shared_ptr<nt::NetworkTable> table);
        TargetInfo(const TargetInfo&) = delete;
        TargetInfo(TargetInfo&&) = default;
        ~TargetInfo() = default;

        /**
         * Set the current target using it's name
         * @param target The target's name
        */
        void setTarget(const std::string& target);
        /**
         * Checks if enough time has passed since the last call to setTarget() to reset the active target to "none"
         * @param ltime loop time of a processing instance (this should be called each frame within an instance)
        */
        bool update(double ltime);

    private:
        const std::shared_ptr<nt::NetworkTable> ttable;
        CHRONO::high_resolution_clock::time_point last;

    } active_target{vision};

};

}   // namespace vs1

#include "visionserver.inc"