//
// Created by afischer on 03.03.21.
//

#ifndef TIMERINTERFACE_H_
#define TIMERINTERFACE_H_

#include <cstdint>
namespace Timer2 {

class TimeInterface {
 public:
  TimeInterface(int64_t startTime);
  ~TimeInterface() = default;
  virtual int64_t getTime() = 0;

 private:


 protected:
  int64_t _startTime;

};

class SimTime: public TimeInterface {

 public:
  SimTime(int64_t startTime, bool init);
  int64_t getTime() override;

 private:
  bool _init;
  int64_t _lastTime;

};

class SysTime: public TimeInterface{
 public:
  SysTime();
  int64_t getTime() override;

};

}


#endif
