#include <vector>

#include "tools/src/resources.h"
#include "tools/src/sighandle.h"
#include "tools/src/timing.h"

#include "api/weightedsubtraction.h"
#include "api/visioncamera.h"
#include "api/visionserver.h"
#include "api/processing.h"
#include "api/vision.h"

#include "2021/testing.h"
#include "2022/rapidreact.h"

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

	VisionServer server(std::move(cameras));
	//server.runVision<SquareTargetPNP, TargetSolver<Test6x6, WeightedSubtraction<VThreshold::LED::BLUE> >, TargetSolver<Test6x6, WeightedSubtraction<VThreshold::LED::GREEN> > >(25);
	server.runVision<CargoFinder, StripFinder<VThreshold::LED::GREEN> >(25);
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
*/