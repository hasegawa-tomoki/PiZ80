#include "bus.hpp"
#include <circle/timer.h>

void Bus::waitNanoSec(unsigned ns){
    CTimer::Get()->nsDelay(ns);
}
