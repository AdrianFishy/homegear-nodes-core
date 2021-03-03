//
// Created by afischer on 03.03.21.
//

#include "TimerInterfaceFactory.h"



Timer2::TimeInterface *Timer2::TimerInterfaceFactory::getInstance(int64_t figureKey) {

    if (figureKey > 0) {
        return new SimTime(figureKey, true);
    } else {
        return new SysTime();
    }

}
