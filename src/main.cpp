#include "opencv2/core/types.hpp"

#include "extras/resources.h"
#include "extras/sighandle.h"
#include "extras/timing.h"

//#define SWITCHED_CAMERA_CONFIG // enable this to setup switched cameras from the config file, otherwise the program starts a single switched camera for vision

#include "visioncamera.h"
#include "pipelines.h"
#include "visionserver.h"
#include "mem.h"

StopWatch runtime("Runtime", &std::cout, 0);
void on_exit() {
	runtime.end();
}

int main(int argc, char* argv[]) {
	runtime.setStart();
	SigHandle::get();
	atexit(on_exit);

	std::vector<VisionCamera> cameras;

	if(argc > 1 && readConfig(cameras, argv[1])) {}
	else if(readConfig(cameras)) {}
	else { return EXIT_FAILURE; }

	// cv::Mat_<double> cam_matx = cv::Mat_<double>(3, 3), cam_dist = cv::Mat_<double>(1, 5);
	// for(size_t i = 0; i < cameras.size(); i++) {
	// 	if(cameras[i].getCameraMatrix(cam_matx)) {
	// 		std::cout << "Found camera matrix for '" << cameras[i].GetName() << "':\n" << cam_matx << "\n\n";
	// 	} else {
	// 		std::cout << "Failed to find camera matrix for '" << cameras[i].GetName() << "'\n\n";
	// 	}
	// 	if(cameras[i].getDistortion(cam_dist)) {
	// 		std::cout << "Found distortion for '" << cameras[i].GetName() << "':\n" << cam_dist << "\n\n";
	// 	} else {
	// 		std::cout << "Failed to find distortion for '" << cameras[i].GetName() << "'\n\n";
	// 	}
	// }

	VisionServer server(cameras);
	server.runVision<SquareTargetPNP, BBoxDemo, DefaultPipeline>(25);
}

// LIST OF THINGS
/*
x? Dynamic resizing/scaling
- Position math -> networktables
x? GET NETWORKTABLES TO WORK
x multiple cameras -> switching (find out what we want to do)
x compression/stay under bandwidth limit
x? Modularize?
- MORE CUSTOM ASSEMBLY!!! :)
*/