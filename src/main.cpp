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
#include "core/tfmodel.h"

#include "2022/rapidreact2.h"


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
	vs2::VisionServer::addStream("stream");
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