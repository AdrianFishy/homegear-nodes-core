//
// Created by afischer on 25.02.21.
//

#include <homegear-node/Math.h>
#include "SimulateTime.h"


namespace Timer2 {

SimulateTime::SimulateTime() {
_init = true;
}

SimulateTime::~SimulateTime() {

}

int64_t SimulateTime::getSimulatedTime(int64_t utcTime) {
    time_t t;
    time_t diff;
    time_t start_time;
    time_t time_since_start;

    if (_init){
        const auto timePoint = std::chrono::system_clock::now();
        t = std::chrono::system_clock::to_time_t(timePoint);

        std::tm localTime{};
        localtime_r(&t, &localTime);
        int64_t millisecondOffset = localTime.tm_gmtoff * 1000;

        start_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() + millisecondOffset;
        _last_time = start_time;
        _init = false;
    }else{
        start_time = _last_time;
    }

    const auto timePoint = std::chrono::system_clock::now();
    t = std::chrono::system_clock::to_time_t(timePoint);

    std::tm localTime{};
    localtime_r(&t, &localTime);
    int64_t millisecondOffset = localTime.tm_gmtoff * 1000;
    time_since_start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() + millisecondOffset;

    diff = time_since_start - start_time;

    return utcTime + diff;
    }


}