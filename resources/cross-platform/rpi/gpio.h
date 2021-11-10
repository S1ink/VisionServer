#pragma once

#ifdef __unix__

#include "pigpio.h"

#pragma message "Make sure this header is only used for raspberry pi"

// this code is somewhat useless...

namespace gpin {
	constexpr uint16_t pi_fan = 18;
	constexpr uint16_t pi_power = 3;

	constexpr uint16_t pc_power = 20;
	constexpr uint16_t pc_reset = 16;
	constexpr uint16_t pc_status = 12;
}

int getStatus();
int getSwitch();
void activateSwitch(int id);
void init(float fanspeed = 40.f);
void setNoctua(float percent);

#else
#pragma message "GPIO is only inteneded for raspberry pi, this header should not be included!"
#endif