/* Copyright 2013-2019 Homegear GmbH
 *
 * Homegear is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Homegear is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Homegear.  If not, see <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */

#ifndef MYNODE_H_
#define MYNODE_H_

#include "SunTime.h"
#include "SimulateTime.h"
#include <homegear-node/INode.h>
#include <thread>
#include <mutex>


namespace Timer2 {

class MyNode : public Flows::INode {
 public:
  MyNode(const std::string &path, const std::string &type, const std::atomic_bool *frontendConnected);
  ~MyNode() override;

  bool init(const Flows::PNodeInfo &info) override;
  bool start() override;
  void startUpComplete() override;
  void stop() override;
  void waitForStop() override;



 private:
  SunTime _sunTime;
  SimulateTime _simTime;
  std::atomic_bool _enabled{true};
  bool _outputOnStartUp = false;
  std::mutex _timeVariableMutex;
  double _latitude = 54.32;
  double _longitude = 10.13;
  struct NextTime{
     int64_t time;
     int64_t day;
     int64_t month;
     int year;
  };

  std::string _type;
  std::string _trigger;
  int64_t _onOffset;
  std::string _timepoint;
  int64_t _period;
  std::string _daysdaily;
  int64_t _lastTime;
  std::mutex _timerMutex;
  std::atomic_bool _stopThread{true};
  std::atomic_bool _stopped{true};
  std::atomic_bool _forceUpdate{false};
  std::thread _timerThread;
  std::vector<bool> _weekdays;
  std::vector<bool> _months;
  std::vector<bool> _days;
  int64_t _current_time = 0;

 private:
  std::tm _tm{};
  time_t _simulated_time = 0;

  std::vector<std::string> splitAll(std::string string, char delimiter);
  void timer();
  int64_t getSunTime(int64_t timeStamp, const std::string& time);
  int64_t getTime(int64_t currentTime, const std::string& time, const std::string& timeType, int64_t offset);
  void input(const Flows::PNodeInfo &info, uint32_t index, const Flows::PVariable &message) override;
  NextTime getNext();
  void printNext(NextTime next);
  std::vector<int> BoolVectorToIntVector(std::vector<bool> boolVal);
  int GetDaysMaxCurrentMonth();
  int GetDaysMaxThisMonth(int month);
  std::vector<int32_t> SplitStringToIntVector (std::string string_to_split);
  int GetOffsetWeekday (int current_weekday, int next);
  int SearchForHigherOrEqualNumber(std::vector<int32_t> &vectors, int day_max, int number, int valueForNoResult);
  int SearchForSmallerNumber(std::vector<int32_t> &vectors, int day_max, int number, int valueForNoResult);
  int SearchForHigherNumber(std::vector<int32_t> &vectors, int day_max, int number, int valueForNoResult);
};

}

#endif
