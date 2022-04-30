#include <vector>
#include <string>

#include "tools/src/resources.h"
#include "tools/src/sighandle.h"
#include "tools/src/timing.h"
#include "tools/src/server/server.h"

#include "core/weightedsubtraction.h"
#include "core/visioncamera.h"
#include "core/visionserver.h"
#include "core/processing.h"
#include "core/vision.h"
#include "core/httpnetworktables.h"

#include "2021/testing.h"
#include "2022/rapidreact.h"


StopWatch runtime("Runtime", &std::cout, 0);
void on_exit() { runtime.end(); }

template<class derived = void>	// each different pipeline type has it's own instance count
class VPipeline2 {
public:
	inline VPipeline2(const std::string& name) : 
		name(name + std::to_string(instances + 1))/*, table(VisionServer::getTable().getSubTable(this->name))*/
	{ instances++; }

	inline static size_t getInstances() { return instances; }
	inline const std::string& getName() { return this->name; }

private:
	std::string name;
	const std::shared_ptr<nt::NetworkTable> table;

	//inline static std::atomic<uint32_t> instances{0};
	inline static uint32_t instances{0};


};

template<class derived = void>
class T1 : public VPipeline2<T1<derived> > {
public:
	typedef VPipeline2<T1<derived> >	Super;
	inline T1() : Super("T1") { std::cout << "T1: " << this->getInstances() << newline; }
	inline T1(const std::string& n) : Super(n) {}


};
template<class derived = void>
class T2 : public T1<T2<derived> > {
public:
	typedef T1<T2<derived> >	Super;
	inline T2() : Super("T2") { std::cout << "T2: " << this->getInstances() << newline; }


};

int main(int argc, char* argv[]) {
	runtime.setStart();
	SigHandle::get();
	atexit(on_exit);

	std::vector<VisionCamera> cameras;

	if(argc > 1 && readConfig(cameras, argv[1])) {}
	else if(readConfig(cameras)) {}
	else { return EXIT_FAILURE; }

	T1 one;
	std::cout << one.getName() << newline;
	T2 two;
	std::cout << two.getName() << newline;
	T2 three;
	std::cout << three.getName() << newline;
	std::cout << "\nT1: " << T1<>::getInstances() << ", T2: " << T2<>::getInstances() << "\n\n";
	std::cout << VPipeline2<>::getInstances() << newline;
	VPipeline2 v("test");
	std::cout << VPipeline2<>::getInstances() << newline;

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
	//testCapture(vserver);
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