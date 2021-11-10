#include "gpio.h"

int getStatus() {
    return gpioRead(gpin::pc_status);
}

int getSwitch() {
    return gpioRead(gpin::pi_power);
}

void activateSwitch(int gpio) {
    gpioWrite(gpio, 1);
    gpioDelay(250000);
    gpioWrite(gpio, 0);
}

void setNoctua(float percent) {
    gpioHardwarePWM(gpin::pi_fan, 25000, (10000 * percent));
}

void init(float fanspeed) {
    gpioInitialise();
    gpioSetMode(gpin::pc_power, PI_OUTPUT);
    gpioSetMode(gpin::pc_reset, PI_OUTPUT);
    gpioSetMode(gpin::pc_status, PI_INPUT);
    gpioSetMode(gpin::pi_power, PI_INPUT);
    gpioHardwarePWM(gpin::pi_fan, 25000, (fanspeed * 10000));
}