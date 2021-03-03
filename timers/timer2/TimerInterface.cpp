//
// Created by afischer on 03.03.21.
//
#include <ctime>
#include <chrono>
#include "TimerInterface.h"

namespace Timer2 {

SysTime::SysTime() : TimeInterface(0) {};

SimTime::SimTime(int64_t startTime, bool init): TimeInterface(startTime), _init(init) {}

TimeInterface::TimeInterface(int64_t startTime){
    _startTime = startTime;
}

int64_t SimTime::getTime() {
    time_t diff;
    time_t startTime;
    time_t timeSinceStart;

    if (_init) {
        startTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        _lastTime = startTime;
        _init = false;
    } else {
        startTime = _lastTime;
    }

    timeSinceStart = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    diff = timeSinceStart - startTime;

    return diff + _startTime;

}

int64_t SysTime::getTime() {
    time_t time;
    time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    return time;

}


}