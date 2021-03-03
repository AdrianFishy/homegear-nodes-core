//
// Created by afischer on 03.03.21.
//
#include <ctime>
#include <chrono>
#include "TimerInterface.h"

namespace Timer2 {

SysTime::SysTime() : TimeInterface(0) {};

SimTime::SimTime(int64_t starttime, bool init): TimeInterface(starttime), _init(init) {}

TimeInterface::TimeInterface(int64_t starttime){
    _starttime = starttime;
}

int64_t SimTime::GetTime() {
    time_t diff;
    time_t start_time;
    time_t time_since_start;

    if (_init) {
        start_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        _last_time = start_time;
        _init = false;
    } else {
        start_time = _last_time;
    }

    time_since_start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    diff = time_since_start - start_time;

    return diff + _starttime;

}

int64_t SysTime::GetTime() {
    time_t time;
    time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    return time;

}


}