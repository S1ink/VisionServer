#include "opencv2/core/types.hpp"

#include "extras/resources.h"
#include "extras/sighandle.h"
#include "extras/timing.h"

//#define SWITCHED_CAMERA_CONFIG	// enable this to setup switched cameras from the config file, otherwise the program starts a single switched camera for vision

#include "visioncamera.h"
#include "pipelines.h"
#include "visionserver.h"

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

	VisionServer server(cameras);
	server.runVision<DefaultPipeline, BBoxDemo, SquareSolver>(25);
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
- Target abstraction and generalization (pipeline template param)
- System for telling the robot when targeting info is outdated
*/