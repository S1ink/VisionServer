#pragma once

#include <iostream>
#include <chrono>
#include <ctime>

#include "resources.h"
#include "output.h"

#ifdef _WIN32
#pragma push_macro("min")
#undef min
#endif

//time_t now();
//tm* struct_now();
//double perfTimer(CHRONO::time_point<CHRONO::high_resolution_clock>& start);

class StopWatch {
private:
	CHRONO::time_point<CHRONO::high_resolution_clock> ref;
	uint8_t settings;
	olstream buff, * output;
	const char* scope;
public:
	static constexpr uint8_t ON_EXIT = 2;	// 1L << 1 (second bit)
	static constexpr uint8_t NOW = 1;		// 1L << 0 (first bit)

	StopWatch(olstream&& out = &std::cout, uint8_t settings = (ON_EXIT | NOW));
	StopWatch(const char* scope, const olstream& out = &std::cout, uint8_t settings = (ON_EXIT | NOW));
	StopWatch(const char* scope, olstream&& out = &std::cout, uint8_t settings = (ON_EXIT | NOW));
	StopWatch(const char* scope, olstream* out, uint8_t settings = (ON_EXIT | NOW));
	~StopWatch();

	void setStart();
	double getDuration();
	void print();
	void print(olstream&& out);
	void end();
	void end(olstream&& out);
};

class ScopeTimer : public StopWatch {	//make protected?
public:
	ScopeTimer(olstream&& out = &std::cout) : StopWatch(std::move(out), (StopWatch::NOW | StopWatch::ON_EXIT)) {}
	ScopeTimer(const char* scopename, olstream&& out = &std::cout) : StopWatch(scopename, std::move(out), (StopWatch::NOW | StopWatch::ON_EXIT)) {}
};

typedef unsigned int time_d;
time_d createTOD(int16_t hr, int16_t min, int16_t sec);

struct DayTime {
	int16_t hr;
	int16_t min;
	int16_t sec;

	DayTime(const int16_t hr = 0, const int16_t min = 0, const int16_t sec = 0) : hr(hr), min(min), sec(sec) {}

	void update();
	//static void update(DayTime& tme);

	bool isRelative();
	time_d toTOD();
};

time_t untilDayTime(const DayTime& tme);
CHRONO::time_point<CHRONO::system_clock> nextDayTime(const DayTime& tme);

#ifdef _WIN32
#pragma pop_macro("min")
#endif