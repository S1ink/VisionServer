#include <vector>
#include <string>

#include "tools/src/resources.h"
#include "tools/src/sighandle.h"
#include "tools/src/timing.h"
#include "tools/src/server/server.h"

#include "core/weightedsubtraction.h"
#include "core/visioncamera.h"
//#include "core/visionserver.h"
#include "core/processing.h"
#include "core/vision.h"
#include "core/httpnetworktables.h"
#include "core/visionserver2.h"

#include "2021/testing.h"
#include "2022/rapidreact.h"


StopWatch runtime("Runtime", &std::cout, 0);
void on_exit() { runtime.end(); }

class TestPipeline : public VPipeline2<TestPipeline> {
public:
	TestPipeline() : VPipeline2("Test Pipeline") {}


};

int main(int argc, char* argv[]) {
	runtime.setStart();
	SigHandle::get();
	atexit(on_exit);

	std::vector<VisionCamera> cameras;

	if(argc > 1 && readConfig(cameras, argv[1])) {}
	else if(readConfig(cameras)) {}
	else { return EXIT_FAILURE; }

	VisionServer2::addCameras(std::move(cameras));
	VisionServer2::addPipelines<TestPipeline, TestPipeline>();
	VisionServer2::addStreams(2);
	VisionServer2::run(30);
	VisionServer2::test();

	// const VisionCamera* camera = &VisionServer2::getCameras().at(1);

	// cs::MjpegServer out("server 1", 1181);
	// cs::MjpegServer out2("server 2", 1182);
	// cs::CvSink src("cv_1");
	// cs::CvSink src2("cv_2");
	// src.SetSource(*camera);
	// src2.SetSource(*camera);
	// cs::CvSource s("CvSource 1", cs::VideoMode());
	// cs::CvSource s2("CvSource 2", cs::VideoMode());
	// //src2.SetSource(s);
	// out.SetSource(s);
	// out2.SetSource(s2);
	// cv::Mat 
	// 	frame = cv::Mat::zeros(VisionServer2::getCameras().at(1).getResolution(), CV_8UC3),
	// 	frame2 = cv::Mat::zeros(VisionServer2::getCameras().at(1).getResolution(), CV_8UC3)
	// ;

	// s.SetVideoMode(camera->GetVideoMode());
	// s2.SetVideoMode(camera->GetVideoMode());

	// for(;;) {
	// 	src.GrabFrame(frame);
	// 	s.PutFrame(frame);
	// 	src2.GrabFrame(frame2);
	// 	s2.PutFrame(frame2);
	// }

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