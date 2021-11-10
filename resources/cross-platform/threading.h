#pragma once

#include <thread>
#include <chrono>
#include <atomic>

#include "resources.h"
#include "timing.h"

// TODO: refresh api with CHRONO support

template<typename return_t, typename... args_t>
void routineThread(const std::atomic_bool& control, const DayTime tme, const time_t uintv, return_t(*func)(args_t...), args_t... args) {
	std::atomic_bool& con = const_cast<std::atomic_bool&>(control);
	while (con) {
		while (uintv < untilDayTime(tme)) {
			if (!con) {
				break;
			}
			std::this_thread::sleep_for(CHRONO::seconds(uintv));
		}
		if (con) {
			std::this_thread::sleep_until(nextDayTime(tme));
			func(args...);
			std::this_thread::sleep_for(CHRONO::seconds(1));
		}
	}
}

// may need to rename to distinguish from ^
template<typename return_t, typename... args_t>
void p_routineThread(std::atomic_bool** control, const DayTime tme, const time_t uintv, return_t(*func)(args_t...), args_t... args) {
	while (**control) {
		while (uintv < untilDayTime(tme)) {
			if (!**control) {
				break;
			}
			std::this_thread::sleep_for(CHRONO::seconds(uintv));
		}
		if (**control) {
			std::this_thread::sleep_until(nextDayTime(tme));
			func(args...);
			std::this_thread::sleep_for(CHRONO::seconds(1));
		}
	}
}

// >> another version of the above but with CHRONO time implemented <<

//this function starts the interval to wait after the desired func finishes, so routines will be altered by however long the function takes
template<typename d_rep, typename d_period, typename return_t, typename... args_t>
void loopingThread(std::atomic_bool const& control, CHRONO::duration<d_rep, d_period> interval, CHRONO::duration<d_rep, d_period> uinterval, return_t(*func)(args_t...), args_t... args) {
	std::atomic_bool& con = const_cast<std::atomic_bool&>(control);
	int ratio = CHRONO::duration_cast<CHRONO::seconds>(interval) / CHRONO::duration_cast<CHRONO::seconds>(uinterval);
	if (ratio > 1) {
		CHRONO::duration<d_rep, d_period> rest = interval % uinterval;
		while (con) {
			for (int i = 0; i < ratio; i++) {
				if (!con) {
					break;
				}
				std::this_thread::sleep_for(uinterval);
			}
			if (con) {
				std::this_thread::sleep_for(rest);
				func(args...);
			}
		}
	}
	else {
		while (con) {
			std::this_thread::sleep_for(interval);
			func(args...);
		}
	}
}