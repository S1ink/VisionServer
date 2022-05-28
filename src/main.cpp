#include <vector>
#include <array>
#include <string>
#include <sstream>
#include <thread>

#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/interpreter_builder.h>
#include <tensorflow/lite/model_builder.h>
#include <tensorflow/lite/kernels/register.h>
#include <edgetpu.h>
#include <opencv2/opencv.hpp>

#include "tools/src/resources.h"
#include "tools/src/sighandle.h"
#include "tools/src/timing.h"
#include "tools/src/types.h"
#include "tools/src/server/server.h"

#include "core/visioncamera.h"
#include "core/vision.h"
//#include "core/httpnetworktables.h"
#include "core/visionserver2.h"

//#include "2022/rapidreact.h"
#include "2022/model_runner.h"


class PoseNet : public vs2::VPipeline<PoseNet> {
public:
	inline static const char* default_model = "lite-model_movenet_singlepose_lightning_tflite_int8_4.tflite";
	enum class Outputs {
		NOSE,
		L_EYE, R_EYE,
		L_EAR, R_EAR,
		L_SHOULDER, R_SHOULDER,
		L_ELBOW, R_ELBOW,
		L_WRIST, R_WRIST,
		L_HIP, R_HIP,
		L_KNEE, R_KNEE,
		L_ANKLE, R_ANKLE,
		TOTAL
	};
	inline static const std::array<const char*, (size_t)Outputs::TOTAL> names{
		"nose",
		"left eye", "right eye",
		"left ear", "right ear",
		"left shoulder", "right shoulder",
		"left elbow", "right elbow",
		"left wrist", "right wrist",
		"left hip", "right hip",
		"left knee", "right knee",
		"left ankle", "right ankle"
	};

	PoseNet(size_t threads = std::thread::hardware_concurrency() / 2) :
		PoseNet(default_model, threads) {}
	PoseNet(const char* modelf, size_t threads = std::thread::hardware_concurrency() / 2) :
		VPipeline("PoseNet Runner")
	{
		this->model = tflite::FlatBufferModel::BuildFromFile(modelf);
		if(!this->model) {

		}
		if(edgetpu::EdgeTpuManager::GetSingleton()->EnumerateEdgeTpu().size()) {

		} else {
			tflite::InterpreterBuilder builder(*this->model, this->resolver);
			builder.SetNumThreads(threads);
			builder(&this->nnetwork);
		}
		this->nnetwork->AllocateTensors();
		if(this->nnetwork->inputs().size() != 1) {

		}
		this->input_tensor = this->nnetwork->input_tensor(0);
		TfLiteIntArray* arr = this->input_tensor->dims;
		this->input_size = cv::Size(arr->data[1], arr->data[2]);
		if(arr->data[3] != 3) {

		}
		if(this->input_tensor->type != kTfLiteUInt8) {

		}
		this->input_buffer = cv::Mat(this->input_size, CV_8UC3, this->input_tensor->data.data);
		if(this->nnetwork->outputs().size() != 1) {

		}
		this->output_tensor = this->nnetwork->output_tensor(0);

	}

	void connectIf(cv::Mat& io_frame, Outputs a, Outputs b, float* d, float t, const cv::Scalar& s = {25, 255, 0}) {
		if(d[num(a)*3 + 2] > t && d[num(b)*3 + 2] > t) {
			cv::line(io_frame, this->points[num(a)], this->points[num(b)], s);
		}
	}

	virtual void process(cv::Mat& io_frame) override {
		cv::resize(io_frame, this->input_buffer, this->input_size);
		this->nnetwork->Invoke();

		float* data = (float*)this->output_tensor->data.data;
		for(size_t i = 0; i < this->output_tensor->dims->data[2]; i++) {
			this->points[i] = std::move(cv::Point2f(
				data[i * 3 + 1] * io_frame.size().width,
				data[i * 3] * io_frame.size().height
			));
			if(data[i * 3 + 2] > 0.3) {
				cv::circle(io_frame, this->points[i], 1, {25, 255, 0}, 2, cv::LINE_AA);
			}
			// std::stringstream t;
			// t << names[i] << ": " << (uint32_t)(data[i * 3 + 2] * 100.f) << "%";
			// cv::putText(
			// 	io_frame, t.str(), this->points[i],
			// 	cv::FONT_HERSHEY_DUPLEX, 0.4, {25, 255, 0}, 1, cv::LINE_AA
			// );
		}
		this->connectIf(io_frame, Outputs::L_SHOULDER, Outputs::R_SHOULDER, data, 0.3, {25, 255, 0});
		this->connectIf(io_frame, Outputs::L_SHOULDER, Outputs::L_ELBOW, data, 0.3, {25, 255, 0});
		this->connectIf(io_frame, Outputs::L_ELBOW, Outputs::L_WRIST, data, 0.3, {25, 255, 0});
		this->connectIf(io_frame, Outputs::R_SHOULDER, Outputs::R_ELBOW, data, 0.3, {25, 255, 0});
		this->connectIf(io_frame, Outputs::R_ELBOW, Outputs::R_WRIST, data, 0.3, {25, 255, 0});
		this->connectIf(io_frame, Outputs::L_HIP, Outputs::R_HIP, data, 0.3, {25, 255, 0});
		this->connectIf(io_frame, Outputs::L_SHOULDER, Outputs::L_HIP, data, 0.3, {25, 255, 0});
		this->connectIf(io_frame, Outputs::R_SHOULDER, Outputs::R_HIP, data, 0.3, {25, 255, 0});
		this->connectIf(io_frame, Outputs::L_HIP, Outputs::L_KNEE, data, 0.3, {25, 255, 0});
		this->connectIf(io_frame, Outputs::R_HIP, Outputs::R_KNEE, data, 0.3, {25, 255, 0});
		this->connectIf(io_frame, Outputs::L_ANKLE, Outputs::L_KNEE, data, 0.3, {25, 255, 0});
		this->connectIf(io_frame, Outputs::R_ANKLE, Outputs::R_KNEE, data, 0.3, {25, 255, 0});
	}

protected:
	inline static size_t num(Outputs e) { return (size_t)e; }

	std::unique_ptr<tflite::FlatBufferModel> model;
	std::unique_ptr<tflite::Interpreter> nnetwork;
	std::shared_ptr<edgetpu::EdgeTpuContext> edgetpu_context;
	tflite::ops::builtin::BuiltinOpResolver resolver;

	cv::Mat input_buffer;
	cv::Size input_size;
	TfLiteTensor
		*input_tensor,
		*output_tensor
	;
	std::array<cv::Point2f, (size_t)Outputs::TOTAL> points;


};

StopWatch runtime("Runtime", &std::cout, 0);
void on_exit() { runtime.end(); }

int main(int argc, char* argv[]) {
	runtime.setStart();
	SigHandle::get();
	atexit(on_exit);

	std::vector<VisionCamera> cameras;

	if(argc > 1 && readConfig(cameras, argv[1])) {}
	else if(readConfig(cameras)) {}
	else { return EXIT_FAILURE; }

	vs2::VisionServer::Init();
	vs2::VisionServer::addCameras(std::move(cameras));
	//vs2::SequentialPipeline<> s;
	ModelRunner m(4);
	// PoseNet p(4);
	// s.addPipeline(&p);
	// s.addPipeline(&m);
	vs2::VisionServer::addPipeline(&m);
	vs2::VisionServer::addStream("dummy");
	vs2::VisionServer::addStream("stream");
	//VisionServer2::addStreams(2);
	vs2::VisionServer::compensate();
	vs2::VisionServer::run(60);
	atexit(vs2::VisionServer::stopExit);

	//VisionServer vserver(std::move(cameras));
	// HttpServer hserver(
	// 	&std::cout,
	// 	"/home/pi",
	// 	nullptr,
	// 	Version::HTTP_1_1,
	// 	"81"	// the main WPILibPi page uses port 80
	// );
	//vserver.runVision_S<CargoFinder, StripFinder<VThreshold::LED::GREEN> >(25);
	// vserver.runVision<
	// 	CargoFinder,
	// 	StripFinder<VThreshold::LED::BLUE>,
	// 	SquareTargetPNP,
	// 	TargetSolver<Test6x6, WeightedSubtraction<VThreshold::LED::BLUE> >,
	// 	BBoxDemo
	// >(25);
	//hserver.serve<HttpNTables>();
}

// LIST OF THINGS
/*	x = done, x? = kind of done
x Dynamic resizing/scaling
x Position math -> networktables
x Test communication with robot -> target positioning w/ drive program
x multiple cameras -> switching (find out what we want to do)
x compression/stay under bandwidth limit
x Modularize?
? MORE CUSTOM ASSEMBLY!!! :)		<--
x Target abstraction and generalization (pipeline template param)
x System for telling the robot when targeting info is outdated
x Toggle pipeline processing (processing or just streaming)
- Networktables continuity with multiple class instances
x Multiple VisionServer processing instances, data protection/management -> vector of threads?
- Robot-program mode where all settings are determined by robot program over ntables
- Automatically deduce nt-connection mode
x TensorFlow models
- VS2 Targets/ntables output
- Coral Edge TPU delegate support
*/