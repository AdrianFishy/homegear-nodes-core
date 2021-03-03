//
// Created by afischer on 03.03.21.
//

#ifndef TIMERINTERFACEFACTORY_H_
#define TIMERINTERFACEFACTORY_H_

#include "TimerInterface.h"
namespace Timer2 {
class TimerInterfaceFactory {
 public:

  static TimeInterface *getInstance(int64_t figureKey);

};
}
#endif //HOMEGEAR_NODES_CORE_TIMERS_TIMER2_TIMERINTERFACEFACTORY_H_
