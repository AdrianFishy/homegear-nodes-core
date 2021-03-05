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

#include <cstring>
#include "MyNode.h"
#include <sstream>
#include <unordered_set>
#include <thread>
#include <ctime>

namespace Timer2 {

MyNode::MyNode(const std::string &path, const std::string &type, const std::atomic_bool *frontendConnected)
    : Flows::INode(path, type, frontendConnected) {

}

MyNode::~MyNode() {
    _stopThread = true;
}

bool MyNode::init(const Flows::PNodeInfo &info) {
    try {

        auto settingsIterator = info->info->structValue->find("startup");
        //_out->printError(info->info->print());

        if (settingsIterator != info->info->structValue->end())
            _outputOnStartUp = settingsIterator->second->booleanValue;

        settingsIterator = info->info->structValue->find("lat");
        if (settingsIterator != info->info->structValue->end())
            _latitude = Flows::Math::getDouble(settingsIterator->second->stringValue);

        settingsIterator = info->info->structValue->find("lon");
        if (settingsIterator != info->info->structValue->end())
            _longitude = Flows::Math::getDouble(settingsIterator->second->stringValue);

        settingsIterator = info->info->structValue->find("typeSelect");
        if (settingsIterator != info->info->structValue->end())
            _type = settingsIterator->second->stringValue;

        settingsIterator = info->info->structValue->find("trigger");
        if (settingsIterator != info->info->structValue->end())
            _trigger = settingsIterator->second->stringValue;

        settingsIterator = info->info->structValue->find("startoff");
        if (settingsIterator != info->info->structValue->end())
            _onOffset = Flows::Math::getNumber(settingsIterator->second->stringValue);

        settingsIterator = info->info->structValue->find("timepoint");
        if (settingsIterator != info->info->structValue->end())
            _timepoint = settingsIterator->second->stringValue;

        settingsIterator = info->info->structValue->find("period");
        if (settingsIterator != info->info->structValue->end())
            _period = Flows::Math::getNumber(settingsIterator->second->stringValue);

        settingsIterator = info->info->structValue->find("daysdaily");
        if (settingsIterator != info->info->structValue->end())
            _daysDaily = settingsIterator->second->stringValue;

        settingsIterator = info->info->structValue->find("simulatedTime");
        if (settingsIterator != info->info->structValue->end()) {
            _starttime = Flows::Math::getNumber64(settingsIterator->second->stringValue);
        }

        Time_Provider = TimerInterfaceFactory::getInstance(_starttime);
        _currentTime = Time_Provider->getTime();
        _currentTime = _sunTime.getLocalTime(_currentTime);
        _sunTime.getTimeStruct(_tm, _currentTime);
        _tm.tm_year = _tm.tm_year + 1900;
        _tm.tm_mon = _tm.tm_mon + 1;

        _weekdays.reserve(7);
        _months.reserve(12);
        _days.reserve(31);
        _weekdays.resize(7, true);
        _months.resize(12, true);
        _days.resize(31, true);

        settingsIterator = info->info->structValue->find("mon");
        if (settingsIterator != info->info->structValue->end()) _weekdays.at(0) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("tue");
        if (settingsIterator != info->info->structValue->end()) _weekdays.at(1) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("wed");
        if (settingsIterator != info->info->structValue->end()) _weekdays.at(2) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("thu");
        if (settingsIterator != info->info->structValue->end()) _weekdays.at(3) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("fri");
        if (settingsIterator != info->info->structValue->end()) _weekdays.at(4) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("sat");
        if (settingsIterator != info->info->structValue->end()) _weekdays.at(5) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("sun");
        if (settingsIterator != info->info->structValue->end()) _weekdays.at(6) = settingsIterator->second->booleanValue;

        bool reactivate = true;
        for (auto day : _weekdays) {
            if (day) {
                reactivate = false;
                break;
            }
        }
        if (reactivate) {
            for (auto day : _weekdays) {
                day = true;
            }
        }

        settingsIterator = info->info->structValue->find("jan");
        if (settingsIterator != info->info->structValue->end()) _months.at(0) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("feb");
        if (settingsIterator != info->info->structValue->end()) _months.at(1) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("mar");
        if (settingsIterator != info->info->structValue->end()) _months.at(2) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("apr");
        if (settingsIterator != info->info->structValue->end()) _months.at(3) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("may");
        if (settingsIterator != info->info->structValue->end()) _months.at(4) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("jun");
        if (settingsIterator != info->info->structValue->end()) _months.at(5) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("jul");
        if (settingsIterator != info->info->structValue->end()) _months.at(6) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("aug");
        if (settingsIterator != info->info->structValue->end()) _months.at(7) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("sep");
        if (settingsIterator != info->info->structValue->end()) _months.at(8) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("oct");
        if (settingsIterator != info->info->structValue->end()) _months.at(9) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("nov");
        if (settingsIterator != info->info->structValue->end()) _months.at(10) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("dec");
        if (settingsIterator != info->info->structValue->end()) _months.at(11) = settingsIterator->second->booleanValue;

        reactivate = true;
        for (auto month : _months) {
            if (month) {
                reactivate = false;
                break;
            }
        }
        if (reactivate) {
            for (auto month : _months) {
                month = true;
            }
        }

        settingsIterator = info->info->structValue->find("1");
        if (settingsIterator != info->info->structValue->end()) _days.at(0) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("2");
        if (settingsIterator != info->info->structValue->end()) _days.at(1) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("3");
        if (settingsIterator != info->info->structValue->end()) _days.at(2) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("4");
        if (settingsIterator != info->info->structValue->end()) _days.at(3) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("5");
        if (settingsIterator != info->info->structValue->end()) _days.at(4) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("6");
        if (settingsIterator != info->info->structValue->end()) _days.at(5) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("7");
        if (settingsIterator != info->info->structValue->end()) _days.at(6) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("8");
        if (settingsIterator != info->info->structValue->end()) _days.at(7) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("9");
        if (settingsIterator != info->info->structValue->end()) _days.at(8) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("10");
        if (settingsIterator != info->info->structValue->end()) _days.at(9) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("11");
        if (settingsIterator != info->info->structValue->end()) _days.at(10) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("12");
        if (settingsIterator != info->info->structValue->end()) _days.at(11) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("13");
        if (settingsIterator != info->info->structValue->end()) _days.at(12) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("14");
        if (settingsIterator != info->info->structValue->end()) _days.at(13) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("15");
        if (settingsIterator != info->info->structValue->end()) _days.at(14) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("16");
        if (settingsIterator != info->info->structValue->end()) _days.at(15) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("17");
        if (settingsIterator != info->info->structValue->end()) _days.at(16) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("18");
        if (settingsIterator != info->info->structValue->end()) _days.at(17) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("19");
        if (settingsIterator != info->info->structValue->end()) _days.at(18) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("20");
        if (settingsIterator != info->info->structValue->end()) _days.at(19) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("21");
        if (settingsIterator != info->info->structValue->end()) _days.at(20) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("22");
        if (settingsIterator != info->info->structValue->end()) _days.at(21) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("23");
        if (settingsIterator != info->info->structValue->end()) _days.at(22) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("24");
        if (settingsIterator != info->info->structValue->end()) _days.at(23) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("25");
        if (settingsIterator != info->info->structValue->end()) _days.at(24) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("26");
        if (settingsIterator != info->info->structValue->end()) _days.at(25) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("27");
        if (settingsIterator != info->info->structValue->end()) _days.at(26) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("28");
        if (settingsIterator != info->info->structValue->end()) _days.at(27) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("29");
        if (settingsIterator != info->info->structValue->end()) _days.at(28) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("30");
        if (settingsIterator != info->info->structValue->end()) _days.at(29) = settingsIterator->second->booleanValue;
        settingsIterator = info->info->structValue->find("31");
        if (settingsIterator != info->info->structValue->end()) _days.at(30) = settingsIterator->second->booleanValue;

        reactivate = true;
        for (auto day : _days) {
            if (day) {
                reactivate = false;
                break;
            }
        }
        if (reactivate) {
            for (auto day : _days) {
                day = true;
            }
        }

        return true;
    }
    catch (const std::exception &ex) {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch (...) {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    return false;
}

bool MyNode::start() {
    try {
        _stopped = false;
        return true;
    }
    catch (const std::exception &ex) {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch (...) {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    return false;
}

void MyNode::startUpComplete() {
    try {
        std::lock_guard<std::mutex> timerGuard(_timerMutex);
        if (!_enabled) return;
        _stopThread = false;
        if (_timerThread.joinable()) _timerThread.join();
        _timerThread = std::thread(&MyNode::timer, this);
    }
    catch (const std::exception &ex) {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch (...) {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void MyNode::stop() {
    try {
        _stopped = true;
        _stopThread = true;
    }
    catch (const std::exception &ex) {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch (...) {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void MyNode::waitForStop() {
    try {
        std::lock_guard<std::mutex> timerGuard(_timerMutex);
        _stopThread = true;
        if (_timerThread.joinable()) _timerThread.join();
    }
    catch (const std::exception &ex) {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch (...) {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

std::vector<std::string> MyNode::splitAll(std::string string, char delimiter) {
    std::vector<std::string> elements;
    std::stringstream stringStream(string);
    std::string element;
    while (std::getline(stringStream, element, delimiter)) {
        elements.push_back(element);
    }
    if (string.back() == delimiter) elements.emplace_back(std::string());
    return elements;
}

int64_t MyNode::getSunTime(int64_t timeStamp, const std::string &time) {
    try {
        auto sunTimes = _sunTime.getTimesLocal(timeStamp, _latitude, _longitude);
        if (time == "sunrise") return sunTimes.times.at(SunTime::SunTimeTypes::sunrise);
        else if (time == "sunset") return sunTimes.times.at(SunTime::SunTimeTypes::sunset);

    }
    catch (const std::exception &ex) {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch (...) {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    return -1;
}

MyNode::NextTime MyNode::getNext() {
    struct NextTime structNextTime;
    std::string type = _type;
    int64_t offset = _onOffset * 1000 * 60;
    int64_t period = _period;
    std::string daysDaily = _daysDaily;
    int64_t currentTime = _currentTime;
    int64_t inputTime = currentTime - 86400000;
    int64_t sunriseTime = getSunTime(currentTime, "sunrise");
    int64_t sunsetTime = getSunTime(currentTime, "sunset");
    int64_t timepointMin = 10;

    int month = _tm.tm_mon;
    int year = _tm.tm_year;
    int64_t dayStart = currentTime - (currentTime % 86400000);
    int nextDay = 0;
    int weekdayOffset = 0;
    bool gapYear;
    int daysMonthMax = 0;
    enum trigger { sunrise, sunset, timepoint };
    enum trigger triggerType;

    if (_trigger == "sunrise") {
        triggerType = sunrise;
    } else if (_trigger == "sunset") {
        triggerType = sunset;
    } else {
        triggerType = timepoint;
    }

    std::vector<int32_t> timepoint_vector = SplitStringToIntVector(_timepoint);

    if (timepoint_vector.at(0) > 23 || timepoint_vector.at(1) > 59 || timepoint_vector.at(0) < 0 || timepoint_vector.at(1) < 0) {
        timepoint_vector.at(0) = 0;
        timepoint_vector.at(1) = 0;
    }

    timepointMin = (timepoint_vector.at(0) * 3600 + timepoint_vector.at(1) * 60) * 1000;

    if (_tm.tm_wday == 0) {
        _tm.tm_wday = 7; //Korrektur da Son=0, Mon=1,...
    }

    int currentWeekday = _tm.tm_wday;

    daysMonthMax = getDaysMaxCurrentMonth();

    if (period == 1) {
        period = 0;
    }

    if (type == "daily") {

        if (daysDaily == "weekend") {
            if (_tm.tm_wday < 6) {
                weekdayOffset = 6 - _tm.tm_wday;
                period = 0;
            }

        }
        if (daysDaily == "workday") {
            if (_tm.tm_wday >= 6) {
                weekdayOffset = (_tm.tm_wday - 8) * (-1);
                period = 0;
            }
            if (currentTime >= sunriseTime + offset) {
                nextDay = 1;
            } else {
                nextDay = 0;
            }
        }

        if (daysDaily == "everyday") {
            if (currentTime >= sunriseTime + offset) {
                nextDay = 1;
            } else {
                nextDay = 0;
            }
        }

        if (triggerType == 0) {
            if (currentTime >= sunriseTime + offset) {

                if (daysDaily == "everyday") {
                    if (currentTime >= sunriseTime + offset && period == 0) {
                        nextDay = 1;
                    } else {
                        nextDay = 0;
                    }
                }

                structNextTime.time = sunriseTime + offset + (weekdayOffset * 86400000) + (period * 86400000) + nextDay * 86400000;
                structNextTime.day = _tm.tm_mday + weekdayOffset + period + nextDay;
                structNextTime.month = month;
                structNextTime.year = year;

                if (structNextTime.day > daysMonthMax) {

                    do {
                        structNextTime.day = structNextTime.day - getDaysMaxThisMonth(structNextTime.month);

                        structNextTime.month = structNextTime.month + 1;

                        if (structNextTime.month > 12) {
                            structNextTime.month = structNextTime.month - 12;
                            structNextTime.year = year + 1;
                        }

                    } while (structNextTime.day > getDaysMaxThisMonth(structNextTime.month));
                } else {
                    structNextTime.month = _tm.tm_mon;
                }

                return structNextTime;
            }
            if (sunriseTime + offset > currentTime) {
                structNextTime.time = sunriseTime + offset + (weekdayOffset * 86400000) + (period * 86400000);
                structNextTime.day = _tm.tm_mday + weekdayOffset + period;
                structNextTime.month = month;
                structNextTime.year = year;

                if (structNextTime.day > daysMonthMax) {
                    do {
                        structNextTime.day = structNextTime.day - getDaysMaxThisMonth(structNextTime.month);
                        structNextTime.month = structNextTime.month + 1;

                        if (structNextTime.month > 12) {
                            structNextTime.month = structNextTime.month - 12;
                            structNextTime.year = year + 1;
                        }

                    } while (structNextTime.day > getDaysMaxThisMonth(structNextTime.month));
                } else {
                    structNextTime.month = _tm.tm_mon;
                }

                return structNextTime;
            }
        }

        if (triggerType == 1) {
            if (currentTime >= sunsetTime + offset) {

                if (daysDaily == "everyday") {
                    if (currentTime >= sunsetTime + offset && period == 0) {
                        nextDay = 1;
                    } else {
                        nextDay = 0;
                    }
                }

                structNextTime.time = sunsetTime + offset + (weekdayOffset * 86400000) + (period * 86400000) + nextDay * 86400000;
                structNextTime.day = _tm.tm_mday + weekdayOffset + period + nextDay;
                structNextTime.month = month;
                structNextTime.year = year;

                if (structNextTime.day > daysMonthMax) {

                    do {
                        structNextTime.day = structNextTime.day - getDaysMaxThisMonth(structNextTime.month);

                        structNextTime.month = structNextTime.month + 1;

                        if (structNextTime.month > 12) {
                            structNextTime.month = structNextTime.month - 12;
                            structNextTime.year = year + 1;
                        }

                    } while (structNextTime.day > getDaysMaxThisMonth(structNextTime.month));
                } else {
                    structNextTime.month = _tm.tm_mon;
                }

                return structNextTime;
            }
            if (sunsetTime + offset > currentTime) {
                structNextTime.time = sunsetTime + offset + (weekdayOffset * 86400000) + (period * 86400000);
                structNextTime.day = _tm.tm_mday + weekdayOffset + period;
                structNextTime.month = month;
                structNextTime.year = year;

                if (structNextTime.day > daysMonthMax) {
                    do {
                        structNextTime.day = structNextTime.day - getDaysMaxThisMonth(structNextTime.month);
                        structNextTime.month = structNextTime.month + 1;

                        if (structNextTime.month > 12) {
                            structNextTime.month = structNextTime.month - 12;
                            structNextTime.year = year + 1;
                        }

                    } while (structNextTime.day > getDaysMaxThisMonth(structNextTime.month));
                } else {
                    structNextTime.month = _tm.tm_mon;
                }

                return structNextTime;
            }

        }
        if (triggerType == 2) {
            if (currentTime >= dayStart + timepointMin) {

                if (daysDaily == "everyday") {
                    if (currentTime >= dayStart + timepointMin && period == 0) {
                        nextDay = 1;
                    } else {
                        nextDay = 0;
                    }
                }

                structNextTime.time = dayStart + timepointMin + (weekdayOffset * 86400000) + (period * 86400000) + nextDay * 86400000;
                structNextTime.day = _tm.tm_mday + weekdayOffset + period + nextDay;
                structNextTime.month = month;
                structNextTime.year = year;

                if (structNextTime.day > daysMonthMax) {
                    do {
                        structNextTime.day = structNextTime.day - getDaysMaxThisMonth(structNextTime.month);
                        structNextTime.month = structNextTime.month + 1;

                        if (structNextTime.month > 12) {
                            structNextTime.month = structNextTime.month - 12;
                            structNextTime.year = year + 1;
                        }

                    } while (structNextTime.day > getDaysMaxThisMonth(structNextTime.month));
                } else {
                    structNextTime.month = _tm.tm_mon;
                }

                return structNextTime;
            }
            if (dayStart + timepointMin > currentTime) {

                structNextTime.time = dayStart + timepointMin + (weekdayOffset * 86400000) + (period * 86400000);
                structNextTime.day = _tm.tm_mday + weekdayOffset + period;
                structNextTime.month = month;
                structNextTime.year = year;

                if (structNextTime.day > daysMonthMax) {
                    do {
                        structNextTime.day = structNextTime.day - getDaysMaxThisMonth(structNextTime.month);
                        structNextTime.month = structNextTime.month + 1;

                        if (structNextTime.month > 12) {
                            structNextTime.month = structNextTime.month - 12;
                            structNextTime.year = year + 1;
                        }

                    } while (structNextTime.day > getDaysMaxThisMonth(structNextTime.month));
                } else {
                    structNextTime.month = _tm.tm_mon;
                }

                return structNextTime;
            }

        }

    }
    if (type == "weekly") {
        int next = 0;
        int currentMonth = month;
        int maxWeekdays = 7;
        int currentMonthDay = _tm.tm_mday;
        int offsetWeekdays;

        int periodOffset = 0;

        if (period > 1) {
            periodOffset = 0;
        } else {
            periodOffset = 1;
        }

        int periodWeekdays;

        if (period > 1) {
            periodWeekdays = (period - 1) * 7;
        } else {
            periodWeekdays = 0;
        }

        std::vector<int32_t> intVectorWeekdays = boolVectorToIntVector(_weekdays);

        next = searchForHigherOrEqualNumber(intVectorWeekdays, maxWeekdays, currentWeekday, next);

        if (next == 0) {
            next = searchForSmallerNumber(intVectorWeekdays, maxWeekdays, currentWeekday, next);
        }

        if (triggerType == 0) {
            if (currentTime >= sunriseTime + offset) {

                if (next == currentWeekday) {
                    next = searchForHigherNumber(intVectorWeekdays, maxWeekdays, currentWeekday, next);
                }

                if (next == currentWeekday) {
                    next = searchForSmallerNumber(intVectorWeekdays, maxWeekdays, currentWeekday, next);
                }
                if (next < currentWeekday) {
                    offsetWeekdays = getOffsetWeekday(currentWeekday, next);
                    next = currentMonthDay + offsetWeekdays;
                } else {
                    next = currentMonthDay + (next - currentWeekday);
                }

                if (intVectorWeekdays.back() == currentWeekday && intVectorWeekdays.size() == 2) {
                    next = currentMonthDay + maxWeekdays;
                }

                structNextTime.time = sunriseTime + offset;
                structNextTime.day = next + periodWeekdays;
                structNextTime.month = month;
                structNextTime.year = year;

                if (structNextTime.day > daysMonthMax) {
                    do {
                        structNextTime.day = structNextTime.day - getDaysMaxThisMonth(structNextTime.month);

                        structNextTime.month = structNextTime.month + 1;

                        if (structNextTime.month > 12) {
                            structNextTime.month = structNextTime.month - 12;
                            structNextTime.year = structNextTime.year + 1;
                        }

                    } while (structNextTime.day > getDaysMaxThisMonth(structNextTime.month));
                }

                return structNextTime;
            }
            if (sunriseTime + offset > currentTime) {

                if (next < currentWeekday) {
                    offsetWeekdays = getOffsetWeekday(currentWeekday, next);
                    next = currentMonthDay + offsetWeekdays;
                } else {
                    next = currentMonthDay + (next - currentWeekday);
                }

                structNextTime.time = sunriseTime + offset;
                structNextTime.day = next + periodWeekdays;
                structNextTime.month = month;
                structNextTime.year = year;

                if (structNextTime.day > daysMonthMax) {
                    do {
                        structNextTime.day = structNextTime.day - getDaysMaxThisMonth(structNextTime.month);

                        structNextTime.month = structNextTime.month + 1;

                        if (structNextTime.month > 12) {
                            structNextTime.month = structNextTime.month - 12;
                            structNextTime.year = structNextTime.year + 1;
                        }

                    } while (structNextTime.day > getDaysMaxThisMonth(structNextTime.month));
                }

                return structNextTime;

            }
        }
        if (triggerType == 1) {

            if (currentTime >= sunsetTime + offset) {

                if (next == currentWeekday) {
                    next = searchForHigherNumber(intVectorWeekdays, maxWeekdays, currentWeekday, next);
                }

                if (next == currentWeekday) {
                    next = searchForSmallerNumber(intVectorWeekdays, maxWeekdays, currentWeekday, next);
                }
                if (next < currentWeekday) {
                    offsetWeekdays = getOffsetWeekday(currentWeekday, next);
                    next = currentMonthDay + offsetWeekdays;
                } else {
                    next = currentMonthDay + (next - currentWeekday);
                }

                if (intVectorWeekdays.back() == currentWeekday && intVectorWeekdays.size() == 2) {
                    next = next + maxWeekdays;
                }

                structNextTime.time = sunsetTime + offset;
                structNextTime.day = next + periodWeekdays;
                structNextTime.month = month;
                structNextTime.year = year;

                if (structNextTime.day > daysMonthMax) {
                    do {
                        structNextTime.day = structNextTime.day - getDaysMaxThisMonth(structNextTime.month);

                        structNextTime.month = structNextTime.month + 1;

                        if (structNextTime.month > 12) {
                            structNextTime.month = structNextTime.month - 12;
                            structNextTime.year = structNextTime.year + 1;
                        }

                    } while (structNextTime.day > getDaysMaxThisMonth(structNextTime.month));
                }

                return structNextTime;
            }
            if (sunsetTime + offset > currentTime) {

                if (next < currentWeekday) {
                    offsetWeekdays = getOffsetWeekday(currentWeekday, next);
                    next = currentMonthDay + offsetWeekdays;
                } else {
                    next = currentMonthDay + (next - currentWeekday);
                }

                structNextTime.time = sunsetTime + offset;
                structNextTime.day = next + periodWeekdays;
                structNextTime.month = month;
                structNextTime.year = year;

                if (structNextTime.day > daysMonthMax) {
                    do {
                        structNextTime.day = structNextTime.day - getDaysMaxThisMonth(structNextTime.month);

                        structNextTime.month = structNextTime.month + 1;

                        if (structNextTime.month > 12) {
                            structNextTime.month = structNextTime.month - 12;
                            structNextTime.year = structNextTime.year + 1;
                        }

                    } while (structNextTime.day > getDaysMaxThisMonth(structNextTime.month));
                }

                return structNextTime;
            }
        }
        if (triggerType == 2) {
            if (currentTime >= dayStart + timepointMin) {

                if (next == currentWeekday) {
                    next = searchForHigherNumber(intVectorWeekdays, maxWeekdays, currentWeekday, next);
                }

                if (next == currentWeekday) {
                    next = searchForSmallerNumber(intVectorWeekdays, maxWeekdays, currentWeekday, next);
                }
                if (next < currentWeekday) {
                    offsetWeekdays = getOffsetWeekday(currentWeekday, next);
                    next = currentMonthDay + offsetWeekdays;
                } else {
                    next = currentMonthDay + (next - currentWeekday);
                }

                if (intVectorWeekdays.back() == currentWeekday && intVectorWeekdays.size() == 2) {
                    next = next + maxWeekdays;
                }

                structNextTime.time = dayStart + timepointMin;
                structNextTime.day = next + periodWeekdays;
                structNextTime.month = month;
                structNextTime.year = year;

                if (structNextTime.day > daysMonthMax) {
                    do {
                        structNextTime.day = structNextTime.day - getDaysMaxThisMonth(structNextTime.month);

                        structNextTime.month = structNextTime.month + 1;

                        if (structNextTime.month > 12) {
                            structNextTime.month = structNextTime.month - 12;
                            structNextTime.year = structNextTime.year + 1;
                        }

                    } while (structNextTime.day > getDaysMaxThisMonth(structNextTime.month));
                }

                return structNextTime;
            }
            if (dayStart + timepointMin > currentTime) {

                if (next < currentWeekday) {
                    offsetWeekdays = getOffsetWeekday(currentWeekday, next);
                    next = currentMonthDay + offsetWeekdays;
                } else {
                    next = currentMonthDay + (next - currentWeekday);
                }

                structNextTime.time = dayStart + timepointMin;
                structNextTime.day = next + periodWeekdays;
                structNextTime.month = month;
                structNextTime.year = year;

                if (structNextTime.day > daysMonthMax) {
                    do {
                        structNextTime.day = structNextTime.day - getDaysMaxThisMonth(structNextTime.month);
                        structNextTime.month = structNextTime.month + 1;

                        if (structNextTime.month > 12) {
                            structNextTime.month = structNextTime.month - 12;
                            structNextTime.year = structNextTime.year + 1;
                        }
                    } while (structNextTime.day > getDaysMaxThisMonth(structNextTime.month));
                } else {
                    structNextTime.month = month;
                }

                //_out->printError("struct_Next_Time.day " + std::to_string(struct_Next_Time.day));
                //_out->printError("struct_Next_Time.month " + std::to_string(struct_Next_Time.month));
                return structNextTime;
            }
        }
    }
    if (type == "monthly") {
        int next = 0;
        int currentMonthday = _tm.tm_mday;
        int currentMonth = month;
        structNextTime.year = year;

        int periodOffset = 0;

        if (period > 1) {
            periodOffset = 0;
        } else {
            periodOffset = 1;
        }

        int periodMonthdays;

        if (period > 1) {
            periodMonthdays = period - 1;
        } else {
            periodMonthdays = 0;
        }
        std::vector<int32_t> intVectorWeekdays = boolVectorToIntVector(_days);

        next = searchForHigherOrEqualNumber(intVectorWeekdays, daysMonthMax, currentMonthday, next);
        if (next == 0) {
            next = searchForSmallerNumber(intVectorWeekdays, daysMonthMax, currentMonthday, next);
        }

        if (triggerType == 0) {
            if (currentTime >= sunriseTime + offset) {

                if (next < currentMonthday) {
                    structNextTime.month = currentMonth + periodOffset + periodMonthdays;
                } else {
                    structNextTime.month = currentMonth + periodMonthdays;
                }

                if (next == currentMonthday) {
                    next = searchForHigherNumber(intVectorWeekdays, daysMonthMax, currentMonthday, next);
                }

                if (next == currentMonthday) {
                    next = searchForSmallerNumber(intVectorWeekdays, daysMonthMax, currentMonthday, next);
                }

                if (intVectorWeekdays.at(intVectorWeekdays.size() - 1) == currentMonthday) {
                    structNextTime.month = currentMonth + periodOffset + periodMonthdays;
                }

                if (structNextTime.month > 12) {
                    structNextTime.month = structNextTime.month - 12;
                    structNextTime.year = structNextTime.year + 1;
                } else {
                    structNextTime.year = year;
                }

                structNextTime.time = sunriseTime + offset;
                structNextTime.day = next;
                return structNextTime;
            }
            if (sunriseTime + offset > currentTime) {

                structNextTime.time = sunriseTime + offset;
                structNextTime.day = next;

                if (next < currentMonthday) {
                    structNextTime.month = currentMonth + periodOffset + periodMonthdays;
                } else {
                    structNextTime.month = currentMonth + periodMonthdays;
                }

                if (structNextTime.month > 12) {
                    structNextTime.month = structNextTime.month - 12;
                    structNextTime.year = structNextTime.year + 1;
                } else {
                    structNextTime.year = year;
                }

                return structNextTime;

            }
        }
        if (triggerType == 1) {

            if (currentTime >= sunsetTime + offset) {

                if (next < currentMonthday) {
                    structNextTime.month = currentMonth + periodOffset + periodMonthdays;
                } else {
                    structNextTime.month = currentMonth + periodMonthdays;
                }

                if (next == currentMonthday) {
                    next = searchForHigherNumber(intVectorWeekdays, daysMonthMax, currentMonthday, next);
                }

                if (next == currentMonthday) {
                    next = searchForSmallerNumber(intVectorWeekdays, daysMonthMax, currentMonthday, next);
                }

                if (intVectorWeekdays.at(intVectorWeekdays.size() - 1) == currentMonthday) {
                    structNextTime.month = currentMonth + periodOffset + periodMonthdays;
                }

                if (structNextTime.month > 12) {
                    structNextTime.month = structNextTime.month - 12;
                    structNextTime.year = structNextTime.year + 1;
                } else {
                    structNextTime.year = year;
                }

                structNextTime.time = sunsetTime + offset;
                structNextTime.day = next;
                return structNextTime;
            }
            if (sunsetTime + offset > currentTime) {

                structNextTime.time = sunsetTime + offset;
                structNextTime.day = next;

                if (next < currentMonthday) {
                    structNextTime.month = currentMonth + periodOffset + periodMonthdays;
                } else {
                    structNextTime.month = currentMonth + periodMonthdays;
                }

                if (structNextTime.month > 12) {
                    structNextTime.month = structNextTime.month - 12;
                    structNextTime.year = structNextTime.year + 1;
                } else {
                    structNextTime.year = year;
                }

                return structNextTime;
            }

        }
        if (triggerType == 2) {
            if (currentTime >= dayStart + timepointMin) {

                if (next < currentMonthday) {
                    structNextTime.month = currentMonth + periodOffset + periodMonthdays;
                } else {
                    structNextTime.month = currentMonth + periodMonthdays;
                }

                if (next == currentMonthday) {
                    next = searchForHigherNumber(intVectorWeekdays, daysMonthMax, currentMonthday, next);
                }

                if (next == currentMonthday) {
                    next = searchForSmallerNumber(intVectorWeekdays, daysMonthMax, currentMonthday, next);
                }

                if (intVectorWeekdays.at(intVectorWeekdays.size() - 1) == currentMonthday) {
                    structNextTime.month = currentMonth + periodOffset + periodMonthdays;
                }

                if (structNextTime.month > 12) {
                    structNextTime.month = structNextTime.month - 12;
                    structNextTime.year = structNextTime.year + 1;
                } else {
                    structNextTime.year = year;
                }

                structNextTime.time = dayStart + timepointMin;
                structNextTime.day = next;
                return structNextTime;
            }
            if (dayStart + timepointMin > currentTime) {

                structNextTime.time = dayStart + timepointMin;
                structNextTime.day = next;

                if (next < currentMonthday) {
                    structNextTime.month = currentMonth + periodOffset + periodMonthdays;
                } else {
                    structNextTime.month = currentMonth + periodMonthdays;
                }

                if (structNextTime.month > 12) {
                    structNextTime.month = structNextTime.month - 12;
                    structNextTime.year = structNextTime.year + 1;
                } else {
                    structNextTime.year = year;
                }
                return structNextTime;
            }

        }
    }
    if (type == "yearly") {
        int nextDayYearly = 0;
        int nextMonth = 0;
        int currentMonthDay = _tm.tm_mday;
        int currentMonth = month;
        int maxMonths = 12;
        int periodOffset = 0;

        if (period > 1) {
            periodOffset = 0;
        } else {
            periodOffset = 1;
        }

        int periodMonthdays;

        if (period > 1) {
            periodMonthdays = period - 1;
        } else {
            periodMonthdays = 0;
        }
        std::vector<int32_t> intVectorMonthdays = boolVectorToIntVector(_days);
        std::vector<int32_t> intVectorMonths = boolVectorToIntVector(_months);

        nextMonth = searchForHigherOrEqualNumber(intVectorMonths, daysMonthMax, currentMonth, nextMonth);

        if (nextMonth == 0) {
            nextMonth = searchForSmallerNumber(intVectorMonths, daysMonthMax, currentMonth, nextMonth);
        }

        nextDayYearly = searchForHigherOrEqualNumber(intVectorMonthdays, daysMonthMax, currentMonthDay, nextDayYearly);

        if (nextDayYearly == 0 || nextMonth != currentMonth) {
            nextDayYearly = searchForSmallerNumber(intVectorMonthdays, daysMonthMax, currentMonthDay, nextDayYearly);
        }

        if (triggerType == 0) {
            if (currentTime >= sunriseTime + offset) {

                if (nextDayYearly == currentMonthDay) {
                    nextDayYearly = searchForHigherNumber(intVectorMonthdays, daysMonthMax, nextDayYearly, nextDayYearly);
                }

                if (nextDayYearly == currentMonthDay) {
                    nextDayYearly = searchForSmallerNumber(intVectorMonthdays, daysMonthMax, nextDayYearly, nextDayYearly);
                }

                if (nextMonth == currentMonth && nextDayYearly < currentMonthDay) {
                    nextMonth = searchForHigherNumber(intVectorMonths, daysMonthMax, nextMonth, nextMonth);
                }

                if (nextMonth == currentMonth && nextDayYearly < currentMonthDay) {
                    nextMonth = searchForSmallerNumber(intVectorMonths, daysMonthMax, nextMonth, nextMonth);
                }
                if (nextDayYearly < currentMonthDay) {
                    structNextTime.month = nextMonth + periodOffset + periodMonthdays;
                } else {
                    structNextTime.month = nextMonth + periodMonthdays;
                }

                if (nextMonth < currentMonth) {

                    structNextTime.month = nextMonth;
                    structNextTime.year = year + periodOffset + periodMonthdays;
                }

                if (nextMonth > currentMonth) {
                    structNextTime.year = year + periodMonthdays;
                    structNextTime.month = nextMonth;
                }

                if (nextDayYearly < currentMonthDay && nextMonth == currentMonth) {
                    structNextTime.year = structNextTime.year + periodOffset;
                }

                if (intVectorMonthdays.at(intVectorMonthdays.size() - 1) == currentMonthDay) {
                    structNextTime.year = year + periodMonthdays + periodOffset;
                }

                if (structNextTime.month > 12) {
                    structNextTime.month = structNextTime.month - 12;
                    structNextTime.year = structNextTime.year + 1;
                }

                structNextTime.time = sunriseTime + offset;
                structNextTime.day = nextDayYearly;
                return structNextTime;
            }
            if (sunriseTime + offset > currentTime) {

                structNextTime.time = sunriseTime + offset;
                structNextTime.day = nextDayYearly;
                structNextTime.month = nextMonth;

                if (nextDayYearly < currentMonthDay) {
                    structNextTime.month = nextMonth;
                } else {
                    structNextTime.month = nextMonth + periodMonthdays;
                }
                if (nextMonth < currentMonth) {

                    structNextTime.month = nextMonth;
                    structNextTime.year = year + periodOffset + periodMonthdays;
                } else {
                    structNextTime.year = year + periodMonthdays;
                    structNextTime.month = nextMonth;
                }

                if (structNextTime.month > 12) {
                    structNextTime.month = structNextTime.month - 12;
                    structNextTime.year = structNextTime.year + 1;
                }
                if (nextDayYearly < currentMonthDay && nextMonth == currentMonth) {
                    structNextTime.year = structNextTime.year + periodOffset + periodMonthdays;
                }

                return structNextTime;

            }
        }
        if (triggerType == 1) {

            if (currentTime >= sunsetTime + offset) {

                if (nextDayYearly == currentMonthDay) {
                    nextDayYearly = searchForHigherNumber(intVectorMonthdays, daysMonthMax, nextDayYearly, nextDayYearly);
                }

                if (nextDayYearly == currentMonthDay) {
                    nextDayYearly = searchForSmallerNumber(intVectorMonthdays, daysMonthMax, nextDayYearly, nextDayYearly);
                }

                if (nextMonth == currentMonth && nextDayYearly < currentMonthDay) {
                    nextMonth = searchForHigherNumber(intVectorMonths, daysMonthMax, nextMonth, nextMonth);
                }

                if (nextMonth == currentMonth && nextDayYearly < currentMonthDay) {
                    nextMonth = searchForSmallerNumber(intVectorMonths, daysMonthMax, nextMonth, nextMonth);
                }

                if (nextDayYearly < currentMonthDay) {
                    structNextTime.month = nextMonth + periodOffset + periodMonthdays;
                } else {
                    structNextTime.month = nextMonth + periodMonthdays;
                }

                if (nextMonth < currentMonth) {
                    structNextTime.month = nextMonth;
                    structNextTime.year = year + periodOffset + periodMonthdays;
                } else {
                    structNextTime.year = year + periodMonthdays;
                    structNextTime.month = nextMonth;
                }

                if (intVectorMonthdays.at(intVectorMonthdays.size() - 1) == currentMonthDay) {
                    structNextTime.year = year + periodMonthdays + periodOffset;
                }

                if (structNextTime.month > 12) {
                    structNextTime.month = structNextTime.month - 12;
                    structNextTime.year = structNextTime.year + 1;
                }

                structNextTime.time = sunsetTime + offset;
                structNextTime.day = nextDayYearly;
                return structNextTime;

            }
            if (sunsetTime + offset > currentTime) {

                structNextTime.time = sunsetTime + offset;
                structNextTime.day = nextDayYearly;
                structNextTime.month = nextMonth;

                if (nextDayYearly < currentMonthDay) {
                    structNextTime.month = nextMonth;
                } else {
                    structNextTime.month = nextMonth + periodMonthdays;
                }
                if (nextMonth < currentMonth) {

                    structNextTime.month = nextMonth;
                    structNextTime.year = year + periodOffset + periodMonthdays;
                } else {
                    structNextTime.year = year + periodMonthdays;
                    structNextTime.month = nextMonth;
                }

                if (structNextTime.month > 12) {
                    structNextTime.month = structNextTime.month - 12;
                    structNextTime.year = structNextTime.year + 1;
                }
                if (nextDayYearly < currentMonthDay && nextMonth == currentMonth) {
                    structNextTime.year = structNextTime.year + periodOffset + periodMonthdays;
                }

                return structNextTime;
            }

        }
        if (triggerType == 2) {
            if (currentTime >= dayStart + timepointMin) {

                if (nextDayYearly == currentMonthDay) {
                    nextDayYearly = searchForHigherNumber(intVectorMonthdays, daysMonthMax, nextDayYearly, nextDayYearly);
                }

                if (nextDayYearly == currentMonthDay) {
                    nextDayYearly = searchForSmallerNumber(intVectorMonthdays, daysMonthMax, nextDayYearly, nextDayYearly);
                }

                if (nextMonth == currentMonth && nextDayYearly < currentMonthDay) {
                    nextMonth = searchForHigherNumber(intVectorMonths, daysMonthMax, nextMonth, nextMonth);
                }

                if (nextMonth == currentMonth && nextDayYearly < currentMonthDay) {
                    nextMonth = searchForSmallerNumber(intVectorMonths, daysMonthMax, nextMonth, nextMonth);
                }

                if (nextDayYearly < currentMonthDay) {
                    structNextTime.month = nextMonth + periodOffset + periodMonthdays;
                } else {
                    structNextTime.month = nextMonth + periodMonthdays;
                }

                if (nextMonth < currentMonth) {

                    structNextTime.month = nextMonth;
                    structNextTime.year = year + periodOffset + periodMonthdays;
                } else {
                    structNextTime.year = year + periodMonthdays;
                    structNextTime.month = nextMonth;
                }

                if (intVectorMonthdays.at(intVectorMonthdays.size() - 1) == currentMonthDay) {
                    structNextTime.year = year + periodMonthdays + periodOffset;
                }

                if (structNextTime.month > 12) {
                    structNextTime.month = structNextTime.month - 12;
                    structNextTime.year = structNextTime.year + 1;
                }

                structNextTime.time = dayStart + timepointMin;
                structNextTime.day = nextDayYearly;
                return structNextTime;
            }

            if (dayStart + timepointMin > currentTime) {

                structNextTime.time = dayStart + timepointMin;
                structNextTime.day = nextDayYearly;
                structNextTime.month = nextMonth;

                if (nextDayYearly < currentMonthDay) {
                    structNextTime.month = nextMonth;
                } else {
                    structNextTime.month = nextMonth + periodMonthdays;
                }
                if (nextMonth < currentMonth) {

                    structNextTime.month = nextMonth;
                    structNextTime.year = year + periodOffset + periodMonthdays;
                } else {
                    structNextTime.year = year + periodMonthdays;
                    structNextTime.month = nextMonth;
                }

                if (structNextTime.month > 12) {
                    structNextTime.month = structNextTime.month - 12;
                    structNextTime.year = structNextTime.year + 1;
                }
                if (nextDayYearly < currentMonthDay && nextMonth == currentMonth) {
                    structNextTime.year = structNextTime.year + periodOffset + periodMonthdays;
                }

                return structNextTime;

            }

        }
    }
}

std::vector<int32_t> MyNode::SplitStringToIntVector(std::string string_to_split) {
    std::vector<int32_t> result;
    result.resize(2);
    auto parts = splitAll(string_to_split, ':');
    if (!string_to_split.empty()) {
        int i = 0;
        for (auto &part : parts) {
            result.at(i) = Flows::Math::getUnsignedNumber(part);
            i++;
        }
    }
    return result;
}

int MyNode::searchForHigherOrEqualNumber(std::vector<int32_t> &vectors, int dayMax, int number, int valueForNoResult) {
    int i;
    for (int day : vectors) {
        if (day > 0 && day <= dayMax) {
            if (day >= number) {
                i = day;
                return i;
            }
        }
    }
    return valueForNoResult;
}

int MyNode::searchForSmallerNumber(std::vector<int32_t> &vectors, int dayMax, int number, int valueForNoResult) {
    int i;
    for (int day : vectors) {
        if (day > 0 && day <= dayMax) {
            if (day < number) {
                i = day;
                return i;
            }
        }
    }
    return valueForNoResult;
}

int MyNode::searchForHigherNumber(std::vector<int32_t> &vectors, int dayMax, int number, int valueForNoResult) {
    int i;
    for (int day : vectors) {
        if (day > 0 && day <= dayMax) {
            if (day > number) {
                i = day;
                return i;
            }
        }
    }
    return valueForNoResult;
}

int MyNode::getDaysMaxCurrentMonth() {
    int month = _tm.tm_mon;
    int year = _tm.tm_year;
    bool gapYear;
    int daysMonthMax = 0;

    if ((year % 100 != 0 && year % 4 == 0) || year % 400 == 0) {
        gapYear = true;
    } else {
        gapYear = false;
    }
    if ((month % 2) == 1) {
        daysMonthMax = 31;
        if (month == 9 || month == 11) {
            daysMonthMax = 30;
        }
    } else {
        daysMonthMax = 30;
        if (month == 8 || month == 10 || month == 12) {
            daysMonthMax = 31;
        }
        if (gapYear && month == 2) {
            daysMonthMax = 29;
        }
        if (!gapYear && month == 2) {
            daysMonthMax = 28;
        }
    }
    return daysMonthMax;
}

int MyNode::getDaysMaxThisMonth(int month) {
    int year = _tm.tm_year;
    bool gapYear;
    int daysMonthMax = 0;

    if ((year % 100 != 0 && year % 4 == 0) || year % 400 == 0) {
        gapYear = true;
    } else {
        gapYear = false;
    }
    if ((month % 2) == 1) {
        daysMonthMax = 31;
        if (month == 9 || month == 11) {
            daysMonthMax = 30;
        }
    } else {
        daysMonthMax = 30;
        if (month == 8 || month == 10 || month == 12) {
            daysMonthMax = 31;
        }
        if (gapYear && month == 2) {
            daysMonthMax = 29;
        }
        if (!gapYear && month == 2) {
            daysMonthMax = 28;
        }
    }
    return daysMonthMax;
}

std::vector<int32_t> MyNode::boolVectorToIntVector(std::vector<bool> boolVal) {
    std::vector<int32_t> intNumbers;
    intNumbers.resize(boolVal.size() + 1);
    int j = 1;
    int c = 1;

    for (int i = 0; i < boolVal.size(); i++) {
        if (boolVal.at(i)) {
            c++;
        }
    }

    for (int i = 0; i < boolVal.size(); i++) {
        if (boolVal.at(i)) {
            intNumbers.at(j) = i + 1;
            j++;
        }
    }
    intNumbers.resize(c);
    return intNumbers;
}

int MyNode::getOffsetWeekday(int currentWeekday, int next) {
    int offsetWeekdays;
    switch (currentWeekday - next) {
        case 1 : offsetWeekdays = 6;
            break;
        case 2 : offsetWeekdays = 5;
            break;
        case 3 : offsetWeekdays = 4;
            break;
        case 4 : offsetWeekdays = 3;
            break;
        case 5 : offsetWeekdays = 2;
            break;
        case 6 : offsetWeekdays = 1;
            break;
    }
    return offsetWeekdays;
}

void MyNode::printNext(NextTime next) {

    int64_t dayNext = next.day;
    int64_t monthNext = next.month;
    int64_t yearNext = next.year;

    next.time = next.time / 1000;

    next.time = next.time % 86400;;
    int32_t hoursNext = next.time / 3600;

    next.time = next.time % 3600;

    int32_t minutesNext = next.time / 60;

    int32_t secondsNext = next.time % 60;

    std::ostringstream timeStream;
    Flows::PVariable status = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);

    timeStream << std::setw(2) << std::setfill('0') << dayNext << "." << std::setw(2) << monthNext << "."
               << std::setw(2) << yearNext << " " << std::setw(2) << hoursNext << ':' << std::setw(2) << minutesNext << ':'
               << std::setw(2) << secondsNext;

    status->structValue->emplace("text", std::make_shared<Flows::Variable>("Next: " + timeStream.str()));
    nodeEvent("statusBottom/" + _id, status, true);

}

void MyNode::timer() {
    int64_t currentTime = _currentTime;
    int64_t lastTime = _currentTime;

    auto nextTime = getNext();
    printNext(nextTime);
    auto test = nextTime;
    bool update = false;
    _lastTime = 86341;

    while (!_stopThread) {
        try {

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            if (_stopThread) {
                break;
            }

            _currentTime = Time_Provider->getTime();
            _currentTime = _sunTime.getLocalTime(_currentTime);
            _sunTime.getTimeStruct(_tm, _currentTime);
            _tm.tm_year = _tm.tm_year + 1900;
            _tm.tm_mon = _tm.tm_mon + 1;

            currentTime = _currentTime;
            currentTime = currentTime / 1000;
            currentTime = currentTime % 86400;

            int64_t timeNext = nextTime.time;
            timeNext = timeNext / 1000;
            timeNext = timeNext % 86400;
            int dayNext = nextTime.day;
            int monthNext = nextTime.month;
            int yearNext = nextTime.year;

            int currentDay = _tm.tm_mday;
            int currentMonth = _tm.tm_mon;
            int currentYear = _tm.tm_year;

            _out->printError("timeNext " + std::to_string(timeNext));
            _out->printError("currentTime " + std::to_string(currentTime));
            _out->printError("dayNext " + std::to_string(dayNext));
            _out->printError("currentDay " + std::to_string(currentDay));
            _out->printError("monthNext " + std::to_string(monthNext));
            _out->printError("currentMonth " + std::to_string(currentMonth));
            _out->printError("currentYear " + std::to_string(currentYear));
            _out->printError("yearNext " + std::to_string(yearNext));
            _out->printError("_lastTime " + std::to_string(_lastTime));
            _out->printError("---------------------------------------");

            _out->printError("_current_time " + std::to_string(_currentTime));

            if (currentTime == timeNext && currentDay == dayNext && currentMonth == monthNext && currentYear == yearNext && _lastTime / 1000 != currentTime / 1000) {
                _lastTime = currentTime;
                update = true;
                setNodeData("lastOnTime", std::make_shared<Flows::Variable>(_lastTime));
                Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                message->structValue->emplace("payload", std::make_shared<Flows::Variable>(true));
                output(0, message);
                _out->printError("---------------------------------------");
                _out->printError("payload = true");
                _out->printError("---------------------------------------");

            }

            if (update || _forceUpdate || currentTime % 3600000 < lastTime % 3600000) //New hour? Recalc in case of time changes or summer/winter time
            {
                update = false;
                _forceUpdate = false;
                nextTime = getNext();

            }
            lastTime = currentTime;
        }
        catch (const std::exception &ex) {
            _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
        }
        catch (...) {
            _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
        }
    }

}

void MyNode::input(const Flows::PNodeInfo &info, uint32_t index, const Flows::PVariable &message) {
    try {
        if (index == 0) //Enabled
        {
            _enabled = message->structValue->at("payload")->booleanValue;
            setNodeData("enabled", std::make_shared<Flows::Variable>(_enabled));
            std::lock_guard<std::mutex> timerGuard(_timerMutex);
            if (_enabled) {
                if (!_stopThread) {
                    _stopThread = true;
                    if (_timerThread.joinable()) _timerThread.join();
                    if (_stopped) return;
                    _stopThread = false;
                    _timerThread = std::thread(&MyNode::timer, this);
                }
            } else {
                _stopThread = true;
                if (_timerThread.joinable()) _timerThread.join();
            }
        }
    }
    catch (const std::exception &ex) {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch (...) {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

}