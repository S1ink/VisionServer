#include <vector>
#include <string>

#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/interpreter_builder.h>
#include <tensorflow/lite/model_builder.h>
#include <tensorflow/lite/kernels/register.h>
#include <tensorflow/lite/examples/label_image/get_top_n.h>

#include "tools/src/resources.h"
#include "tools/src/sighandle.h"
#include "tools/src/timing.h"
#include "tools/src/types.h"
#include "tools/src/server/server.h"

//#include "core/weightedsubtraction.h"
#include "core/visioncamera.h"
//#include "core/visionserver.h"
//#include "core/processing.h"
#include "core/vision.h"
#include "core/httpnetworktables.h"
#include "core/visionserver2.h"

#include "2021/testing.h"
#include "2022/rapidreact.h"
#include "2022/model_runner.h"


StopWatch runtime("Runtime", &std::cout, 0);
void on_exit() { runtime.end(); }

class TestPipeline : public vs2::VPipeline<TestPipeline> {
public:
	TestPipeline() : VPipeline("Test Pipeline") {}


};
class TestPipeline2 : public vs2::VPipeline<TestPipeline2> {
public:
	TestPipeline2() : VPipeline("Test Pipeline[2]") {}


};

int main(int argc, char* argv[]) {
	runtime.setStart();
	SigHandle::get();
	atexit(on_exit);

	std::vector<VisionCamera> cameras;

	if(argc > 1 && readConfig(cameras, argv[1])) {}
	else if(readConfig(cameras)) {}
	else { return EXIT_FAILURE; }

	// std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
	// std::unique_ptr<tflite::FlatBufferModel> model = tflite::FlatBufferModel::BuildFromFile("unoptimized.tflite");

	// tflite::ops::builtin::BuiltinOpResolver resolver;
	// std::unique_ptr<tflite::Interpreter> interpreter;
	// tflite::InterpreterBuilder builder(*model, resolver);
	// builder.SetNumThreads(3);
	// builder(&interpreter);
	// interpreter->AllocateTensors();
	// std::cout << "Loaded TFLITE model in: " << std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - now).count() << " seconds.\n";

	// std::cout <<
	// 	"Model Info: {" <<
	// 		"\n\tInputs: " << interpreter->inputs().size() <<
	// 		"\n\tOutputs: " << interpreter->outputs().size();
	// for(size_t i = 0; i < interpreter->inputs().size(); i++) {
	// 	std::cout << "\n\tInput[" << i << "]: " << interpreter->GetInputName(i) << " ~ " << interpreter->inputs()[i];
	// }
	// for(size_t i = 0; i < interpreter->outputs().size(); i++) {
	// 	std::cout << "\n\tOutput[" << i << "]: " << interpreter->GetOutputName(i) << " ~ " << interpreter->outputs()[i];
	// }
	// // std::cout << "\n}\nTensors:";
	// // for(size_t i = 0; i < interpreter->tensors_size(); i++) {
	// // 	std::cout << "\n\t[" << i << "]: " << interpreter->tensor(i)->bytes << ", " << interpreter->tensor(i)->params.scale;
	// // }
	// // std::cout << std::endl;
	// std::cout << "\n}" << std::endl;

	// int32_t input = interpreter->inputs()[0];
	// TfLiteIntArray* dims = interpreter->tensor(input)->dims;
	// int32_t
	// 	wanted_height = dims->data[1],
	// 	wanted_width = dims->data[2],
	// 	wanted_channels = dims->data[3]
	// ;

	// std::cout << "Model Input Specs: {" << wanted_width << 'x' << wanted_height << ", C" << wanted_channels << "}\n";

	// VisionCamera& cam = cameras.at(1);
	// cs::MjpegServer out("Stream", 1181);
	// cs::CvSink src("cv_1");
	// src.SetSource(cam);
	// cs::CvSource s("Source", cs::VideoMode());
	// out.SetSource(s);
	// s.SetVideoMode(cam.GetVideoMode());

	// cv::Size wanted_size(wanted_width, wanted_height);
	// cv::Mat
	// 	frame = cv::Mat::zeros(cam.getResolution(), CV_8UC3),
	// 	input_buff(wanted_size, CV_8UC(wanted_channels), interpreter->typed_tensor<uint8_t>(input))
	// ;
	// std::vector<std::pair<float, int32_t> > top_results;

	// for(;;) {
	// 	src.GrabFrame(frame);

	// 	cv::resize(frame, input_buff, wanted_size);
	// 	interpreter->Invoke();
	// 	std::cout << "Result produced.\n";
	// 	for(size_t i = 0; i < interpreter->outputs().size(); i++) {
	// 		std::cout << "{ ";
	// 		TfLiteTensor* tensor = interpreter->tensor(interpreter->outputs()[i]);
	// 		// switch(tensor->type) {
	// 		// 	case kTfLiteFloat32: {
	// 		// 		std::cout << "~float~ ";
	// 		// 		break;
	// 		// 	}
	// 		// 	case kTfLiteInt8: {
	// 		// 		std::cout << "~int~ ";
	// 		// 		break;
	// 		// 	}
	// 		// 	case kTfLiteUInt8: {
	// 		// 		std::cout << "~uint~ ";
	// 		// 		break;
	// 		// 	}
	// 		// 	default: {
	// 		// 		std::cout << tensor->type;
	// 		// 	}
	// 		// }
	// 		float* dat = interpreter->typed_output_tensor<float>(i);
	// 		if(tensor && dat) {
	// 			for(int i = 0; i < tensor->dims->data[tensor->dims->size - 1]; i++) {
	// 				std::cout << dat[i] << ", ";
	// 			}
	// 		} else {
	// 			std::cout << (tensor == nullptr) << " : " << (dat == nullptr);
	// 		}
	// 		std::cout << " }\n";
	// 	}
	// 	std::cout << std::endl;

	// 	s.PutFrame(frame);
	// }


	vs2::VisionServer::addCameras(std::move(cameras));
	vs2::VisionServer::addPipelines<ModelRunner>();
	vs2::VisionServer::addStream("dummy");
	vs2::VisionServer::addStream("stream");
	//VisionServer2::addStreams(2);
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
- Multiple VisionServer processing instances, data protection/management -> vector of threads?
- Robot-program mode where all settings are determined by robot program over ntables
- Automatically deduce nt-connection mode
*/