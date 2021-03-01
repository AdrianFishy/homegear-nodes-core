//
// Created by afischer on 25.02.21.
//

#ifndef SIMULATETIME_H_
#define SIMULATETIME_H_

#include <homegear-node/HelperFunctions.h>
#include <unordered_map>
#include <vector>
#include <cmath>



namespace Timer2 {

    class SimulateTime{
     public:
      int64_t getSimulatedTime(int64_t utcTime = 0);
      SimulateTime();
      ~SimulateTime();



     private:

      int64_t _last_time;
      bool _init;


    };
}
#endif
