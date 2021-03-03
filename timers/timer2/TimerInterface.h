//
// Created by afischer on 03.03.21.
//

#ifndef TIMERINTERFACE_H_
#define TIMERINTERFACE_H_

#include <cstdint>
namespace Timer2 {

class TimeInterface {
 public:
  TimeInterface(int64_t starttime);
  ~TimeInterface() = default;
  virtual int64_t GetTime() = 0;

 private:


 protected:
  int64_t _starttime;

};

class SimTime: public TimeInterface {

 public:
  SimTime(int64_t starttime, bool init);
  int64_t GetTime() override;

 private:
  bool _init;
  int64_t _last_time;

};

class SysTime: public TimeInterface{
 public:
  SysTime();
  int64_t GetTime() override;

};

}


#endif
