#include "extras/resources.h"
#include "extras/sighandle.h"
#include "extras/timing.h"

//#define SWITCHED_CAMERA_CONFIG // enable this to setup switched cameras from the config file, otherwise the program starts a single switched camera for vision

#include "visioncamera.h"
#include "visionserver.h"
#include "pipelines.h"
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

	VisionServer server(cameras);
	server.runVision<TestPipeline, PipelineBase>(25);
}

// LIST OF THINGS
/*
x? Dynamic resizing/scaling
- Position math -> networktables
x? GET NETWORKTABLES TO WORK
x multiple cameras -> switching (find out what we want to do)
- compression/stay under bandwidth limit
x? Modularize?
- MORE CUSTOM ASSEMBLY!!! :)
*/