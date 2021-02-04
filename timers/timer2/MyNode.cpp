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
        _out->printError(info->info->print());

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
            _timepoint = Flows::Math::getNumber(settingsIterator->second->stringValue);

        settingsIterator = info->info->structValue->find("period");
        if (settingsIterator != info->info->structValue->end())
            _period = Flows::Math::getNumber(settingsIterator->second->stringValue);

        settingsIterator = info->info->structValue->find("daysdaily");
        if (settingsIterator != info->info->structValue->end())
            _daysdaily = settingsIterator->second->stringValue;

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
            _out->printWarning("Warning: No weekday selected.");
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
            _out->printWarning("Warning: No month selected.");
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
            _out->printWarning("Warning: No day selected.");
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

int64_t MyNode::getTime(int64_t currentTime, const std::string &time, const std::string &timeType, int64_t offset) {
    try {
        if (timeType == "suntime") {
            int32_t i = 0;
            int64_t sunTime = 1;
            int64_t inputTime = currentTime - 86400000;
            while (sunTime + offset < currentTime && sunTime >= 0 && i < 1000) {
                sunTime = getSunTime(inputTime, time);
                inputTime += 86400000;
                i++;
            }
            return sunTime + offset;
        } else {
            auto timeVector = splitAll(time, ':');
            int64_t returnValue = (_sunTime.getLocalTime() / 86400000) * 86400000 + offset - 86400000;
            if (!timeVector.empty()) {
                returnValue += Flows::Math::getNumber64(timeVector.at(0)) * 3600000;
                if (timeVector.size() > 1) {
                    returnValue += Flows::Math::getNumber64(timeVector.at(1)) * 60000;
                    if (timeVector.size() > 2) returnValue += Flows::Math::getNumber64(timeVector.at(2)) * 1000;
                }
            }
            std::tm timeStruct{};
            _sunTime.getTimeStruct(timeStruct);

            return returnValue;
        }
    }
    catch (const std::exception &ex) {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch (...) {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    return 0;
}

MyNode::NextTime MyNode::getNext() {
    struct NextTime structnext_time;
    std::string type_in_getNext = _type;
    std::string trigger_in_getNext = _trigger;
    int64_t offset_in_getNext = _onOffset * 1000 * 60;
    int64_t period_in_getNext = _period;
    std::string daysdaily_in_get_next = _daysdaily;
    int64_t current_time = _sunTime.getLocalTime();
    int64_t inputTime = current_time - 86400000;
    int64_t sunriseTime = getSunTime(current_time, "sunrise");
    int64_t sunsetTime = getSunTime(current_time, "sunset");
    int64_t timepoint_min_in_getNext = _timepoint * 1000 * 60;
    std::tm tm{};
    _sunTime.getTimeStruct(tm);
    int month_in_getNext = tm.tm_mon;
    int year_in_getNext = (tm.tm_year - (month_in_getNext * 100)) + 2000;
    int64_t day_start_in_getNext = current_time - (current_time % 86400000);
    int nextday = 0;
    int weekday_offset = 0;
    bool gap_year;
    int days_mmax = 0;

    tm.tm_mon = tm.tm_mon + 1; //Korrektur da Jan=0,Feb=1,...


    if (tm.tm_wday == 0) {
        tm.tm_wday = 7; //Korrektur da Son=0, Mon=1,...
    }

    int current_weekday = tm.tm_wday;

    days_mmax = GetDaysMax();

    if (period_in_getNext == 1) {
        period_in_getNext = 0;
    }

    if (type_in_getNext == "daily") {

        if (daysdaily_in_get_next == "weekend") {
            if (tm.tm_wday < 6) {
                weekday_offset = 6 - tm.tm_wday;
                period_in_getNext = 0;
            }

        }
        if (daysdaily_in_get_next == "workday") {
            if (tm.tm_wday >= 5) {
                weekday_offset = (tm.tm_wday - 8) * (-1);
                period_in_getNext = 0;
            }
            if (current_time >= sunriseTime + offset_in_getNext) {
                nextday = 1;
            } else {
                nextday = 0;
            }
        }

        if (daysdaily_in_get_next == "everyday") {
            if (current_time >= sunriseTime + offset_in_getNext) {
                nextday = 1;
            } else {
                nextday = 0;
            }
        }

        if (trigger_in_getNext == "sunrise") {
            if (current_time >= sunriseTime + offset_in_getNext) {

                if (daysdaily_in_get_next == "everyday") {
                    if (current_time >= sunriseTime + offset_in_getNext) {
                        nextday = 1;
                    } else {
                        nextday = 0;
                    }
                }

                structnext_time.time = sunriseTime + offset_in_getNext + (weekday_offset * 86400000) + (period_in_getNext * 86400000) + nextday * 86400000;
                structnext_time.day = tm.tm_mday + weekday_offset + period_in_getNext + nextday; // TODO in constante

                if (structnext_time.day > days_mmax) {
                    structnext_time.day = structnext_time.day - days_mmax;
                    structnext_time.month = tm.tm_mon + 1;
                } else {
                    structnext_time.month = tm.tm_mon;
                }

                if (structnext_time.month >= 12) {
                    structnext_time.month = structnext_time.month - 12;
                    structnext_time.year = year_in_getNext + 1;
                } else {
                    structnext_time.year = year_in_getNext;
                }

                return structnext_time;
            }
            if (sunriseTime + offset_in_getNext > current_time) {
                structnext_time.time = sunriseTime + offset_in_getNext + (weekday_offset * 86400000) + (period_in_getNext * 86400000);
                structnext_time.day = tm.tm_mday + weekday_offset + period_in_getNext;

                if (structnext_time.day > days_mmax) {
                    structnext_time.day = structnext_time.day - days_mmax;
                    structnext_time.month = tm.tm_mon + 1;
                } else {
                    structnext_time.month = tm.tm_mon;
                }

                if (structnext_time.month >= 12) {
                    structnext_time.month = structnext_time.month - 12;
                    structnext_time.year = year_in_getNext + 1;
                } else {
                    structnext_time.year = year_in_getNext;
                }

                return structnext_time;
            }
        }

        if (trigger_in_getNext == "sunset") {
            if (current_time >= sunsetTime + offset_in_getNext) {

                if (daysdaily_in_get_next == "everyday") {
                    if (current_time >= sunsetTime + offset_in_getNext) {
                        nextday = 1;
                    } else {
                        nextday = 0;
                    }
                }

                structnext_time.time = sunsetTime + offset_in_getNext + (weekday_offset * 86400000) + (period_in_getNext * 86400000) + nextday * 86400000;
                structnext_time.day = tm.tm_mday + weekday_offset + period_in_getNext + nextday;

                if (structnext_time.day > days_mmax) {
                    structnext_time.day = structnext_time.day - days_mmax;
                    structnext_time.month = tm.tm_mon + 1;
                } else {
                    structnext_time.month = tm.tm_mon;
                }

                if (structnext_time.month >= 12) {
                    structnext_time.month = structnext_time.month - 12;
                    structnext_time.year = year_in_getNext + 1;
                } else {
                    structnext_time.year = year_in_getNext;
                }

                return structnext_time;
            }
            if (sunsetTime + offset_in_getNext > current_time) {
                structnext_time.time = sunsetTime + offset_in_getNext + (weekday_offset * 86400000) + (period_in_getNext * 86400000);
                structnext_time.day = tm.tm_mday + weekday_offset + period_in_getNext;

                if (structnext_time.day > days_mmax) {
                    structnext_time.day = structnext_time.day - days_mmax;
                    structnext_time.month = tm.tm_mon + 1;
                } else {
                    structnext_time.month = tm.tm_mon;
                }

                if (structnext_time.month >= 12) {
                    structnext_time.month = structnext_time.month - 12;
                    structnext_time.year = year_in_getNext + 1;
                } else {
                    structnext_time.year = year_in_getNext;
                }

                return structnext_time;
            }

        }
        if (trigger_in_getNext == "timepoint") {
            if (current_time >= day_start_in_getNext + timepoint_min_in_getNext) {

                if (daysdaily_in_get_next == "everyday") {
                    if (current_time >= day_start_in_getNext + timepoint_min_in_getNext) {
                        nextday = 1;
                    } else {
                        nextday = 0;
                    }
                }

                structnext_time.time = day_start_in_getNext + timepoint_min_in_getNext + (weekday_offset * 86400000) + (period_in_getNext * 86400000) + nextday * 86400000;
                structnext_time.day = tm.tm_mday + weekday_offset + period_in_getNext + nextday;

                if (structnext_time.day > days_mmax) {
                    structnext_time.day = structnext_time.day - days_mmax;
                    structnext_time.month = tm.tm_mon + 1;
                } else {
                    structnext_time.month = tm.tm_mon;
                }

                if (structnext_time.month >= 12) {
                    structnext_time.month = structnext_time.month - 12;
                    structnext_time.year = year_in_getNext + 1;
                } else {
                    structnext_time.year = year_in_getNext;
                }

                return structnext_time;
            }
            if (day_start_in_getNext + timepoint_min_in_getNext > current_time) {
                structnext_time.time = day_start_in_getNext + timepoint_min_in_getNext + (weekday_offset * 86400000) + (period_in_getNext * 86400000);
                structnext_time.day = tm.tm_mday + weekday_offset + period_in_getNext;

                if (structnext_time.day > days_mmax) {
                    structnext_time.day = structnext_time.day - days_mmax;
                    structnext_time.month = tm.tm_mon + 1;
                } else {
                    structnext_time.month = tm.tm_mon;
                }

                if (structnext_time.month >= 12) {
                    structnext_time.month = structnext_time.month - 12;
                    structnext_time.year = year_in_getNext + 1;
                } else {
                    structnext_time.year = year_in_getNext;
                }

                return structnext_time;
            }

        }

    }
    if (type_in_getNext == "weekly") {
        int next = 0;
        int current_Weekday = tm.tm_wday;
        int current_Month = tm.tm_mon;
        int max_weekdays = 7;
        next = 0;
        //TODO Period bei größer als 4 filtern

        int period_offset = 0;

        if (period_in_getNext > 1) {
            period_offset = 0;
        } else {
            period_offset = 1;
        }

        int period_weekdays;

        if (period_in_getNext > 1) {
            period_weekdays = (period_in_getNext - 1) * 7;
        } else {
            period_weekdays = 0;
        }

        std::vector<int32_t> intVectorWeekdays = BoolVectorToIntVector(_weekdays);
        for (int intVectorWeekday : intVectorWeekdays) {
            if (intVectorWeekday > 0 && intVectorWeekday <= max_weekdays) {
                if (intVectorWeekday >= current_Weekday) {
                    next = intVectorWeekday;
                    break;
                }
            }
        }

        if (next == 0) {
            for (int intVectorWeekday : intVectorWeekdays) {
                if (intVectorWeekday > 0 && intVectorWeekday <= max_weekdays) {
                    if (intVectorWeekday < current_Weekday) {
                        next = intVectorWeekday;
                        break;
                    }
                }
            }
        }

        if (trigger_in_getNext == "sunrise") {
            if (current_time >= sunriseTime + offset_in_getNext) {

                if (next == current_Weekday) {
                    for (int intVectorWeekday : intVectorWeekdays) {
                        if (intVectorWeekday > 0 && intVectorWeekday <= max_weekdays) {
                            if (intVectorWeekday > next) {
                                next = intVectorWeekday;
                                break;
                            }
                        }
                    }
                }

                if (next == current_Weekday) {
                    for (int intVectorWeekday : intVectorWeekdays) {
                        if (intVectorWeekday > 0 && intVectorWeekday <= max_weekdays) {
                            if (intVectorWeekday < next) {
                                next = intVectorWeekday;
                                break;
                            }
                        }
                    }
                }
                if (next < current_Weekday) {
                    next = next + max_weekdays;
                }

                if (intVectorWeekdays.back() == current_Weekday && intVectorWeekdays.size() == 1) {
                    next = next + max_weekdays;
                }

                if (next > days_mmax) {
                    next = days_mmax - next;
                    structnext_time.month = structnext_time.month + 1;
                } else {
                    structnext_time.month = current_Month;
                }
                if (structnext_time.month > 12) {
                    structnext_time.month = structnext_time.month - 12;
                    structnext_time.year = structnext_time.year + 1;
                } else {
                    structnext_time.year = year_in_getNext;
                }

                structnext_time.time = sunriseTime + offset_in_getNext;
                structnext_time.day = next + period_weekdays;
                return structnext_time;
            }
            if (sunriseTime + offset_in_getNext > current_time) {

                if (next < current_Weekday) {
                    next = next + max_weekdays;
                }

                if (next > days_mmax) {
                    next = days_mmax - next;
                    structnext_time.month = structnext_time.month + 1;
                } else {
                    structnext_time.month = current_Month;
                }
                if (structnext_time.month > 12) {
                    structnext_time.month = structnext_time.month - 12;
                    structnext_time.year = structnext_time.year + 1;
                } else {
                    structnext_time.year = year_in_getNext;
                }
                structnext_time.time = sunriseTime + offset_in_getNext;
                structnext_time.day = next + period_weekdays;
                return structnext_time;

            }
        }
        if (trigger_in_getNext == "sunset") {

            if (current_time >= sunsetTime + offset_in_getNext) {

                if (next == current_Weekday) {
                    for (int intVectorWeekday : intVectorWeekdays) {
                        if (intVectorWeekday > 0 && intVectorWeekday <= max_weekdays) {
                            if (intVectorWeekday > next) {
                                next = intVectorWeekday;
                                break;
                            }
                        }
                    }
                }

                if (next == current_Weekday) {
                    for (int intVectorWeekday : intVectorWeekdays) {
                        if (intVectorWeekday > 0 && intVectorWeekday <= max_weekdays) {
                            if (intVectorWeekday < next) {
                                next = intVectorWeekday;
                                break;
                            }
                        }
                    }
                }
                if (next < current_Weekday) {
                    next = next + max_weekdays;
                }

                if (intVectorWeekdays.back() == current_Weekday && intVectorWeekdays.size() == 1) {
                    next = next + max_weekdays;
                }

                if (next > days_mmax) {
                    next = days_mmax - next;
                    structnext_time.month = structnext_time.month + 1;
                } else {
                    structnext_time.month = current_Month;
                }
                if (structnext_time.month > 12) {
                    structnext_time.month = structnext_time.month - 12;
                    structnext_time.year = structnext_time.year + 1;
                } else {
                    structnext_time.year = year_in_getNext;
                }

                structnext_time.time = sunsetTime + offset_in_getNext;
                structnext_time.day = next + period_weekdays;
                return structnext_time;
            }
            if (sunsetTime + offset_in_getNext > current_time) {

                if (next < current_Weekday) {
                    next = next + max_weekdays;
                }

                if (next > days_mmax) {
                    next = days_mmax - next;
                    structnext_time.month = structnext_time.month + 1;
                } else {
                    structnext_time.month = current_Month;
                }
                if (structnext_time.month > 12) {
                    structnext_time.month = structnext_time.month - 12;
                    structnext_time.year = structnext_time.year + 1;
                } else {
                    structnext_time.year = year_in_getNext;
                }
                structnext_time.time = sunsetTime + offset_in_getNext;
                structnext_time.day = next + period_weekdays;
                return structnext_time;
            }
        }
        if (trigger_in_getNext == "timepoint") {
            if (current_time >= day_start_in_getNext + timepoint_min_in_getNext) {

                if (next == current_Weekday) {
                    for (int intVectorWeekday : intVectorWeekdays) {
                        if (intVectorWeekday > 0 && intVectorWeekday <= max_weekdays) {
                            if (intVectorWeekday > next) {
                                next = intVectorWeekday;
                                break;
                            }
                        }
                    }
                }

                if (next == current_Weekday) {
                    for (int intVectorWeekday : intVectorWeekdays) {
                        if (intVectorWeekday > 0 && intVectorWeekday <= max_weekdays) {
                            if (intVectorWeekday < next) {
                                next = intVectorWeekday;
                                break;
                            }
                        }
                    }
                }
                if (next < current_Weekday) {
                    next = next + max_weekdays;
                }

                if (intVectorWeekdays.back() == current_Weekday && intVectorWeekdays.size() == 1) {
                    next = next + max_weekdays;
                }

                if (next > days_mmax) {
                    next = days_mmax - next;
                    structnext_time.month = structnext_time.month + 1;
                } else {
                    structnext_time.month = current_Month;
                }
                if (structnext_time.month > 12) {
                    structnext_time.month = structnext_time.month - 12;
                    structnext_time.year = structnext_time.year + 1;
                } else {
                    structnext_time.year = year_in_getNext;
                }

                structnext_time.time = day_start_in_getNext + timepoint_min_in_getNext;
                structnext_time.day = next + period_weekdays;
                return structnext_time;
            }
            if (day_start_in_getNext + timepoint_min_in_getNext > current_time) {

                if (next < current_Weekday) {
                    next = next + max_weekdays;
                }

                if (next > days_mmax) {
                    next = days_mmax - next;
                    structnext_time.month = structnext_time.month + 1;
                } else {
                    structnext_time.month = current_Month;
                }
                if (structnext_time.month > 12) {
                    structnext_time.month = structnext_time.month - 12;
                    structnext_time.year = structnext_time.year + 1;
                } else {
                    structnext_time.year = year_in_getNext;
                }
                structnext_time.time = day_start_in_getNext + timepoint_min_in_getNext;
                structnext_time.day = next + period_weekdays;
                return structnext_time;
            }
        }
    }
    if (type_in_getNext == "monthly") {
        int next = 0;
        int current_Monthday = tm.tm_mday;
        int current_Month = tm.tm_mon;
        next = 0;

        int period_offset = 0;

        if (period_in_getNext > 1) {
            period_offset = 0;
        } else {
            period_offset = 1;
        }

        int period_monthdays;

        if (period_in_getNext > 1) {
            period_monthdays = period_in_getNext - 1;
        } else {
            period_monthdays = 0;
        }
        std::vector<int32_t> intVectorWeekdays = BoolVectorToIntVector(_days);

        for (int intVectorWeekday : intVectorWeekdays) {
            if (intVectorWeekday > 0 && intVectorWeekday <= days_mmax) {
                if (intVectorWeekday >= current_Monthday) {
                    next = intVectorWeekday;
                    break;
                }
            }
        }

        if (next == 0) {
            for (int intVectorWeekday : intVectorWeekdays) {
                if (intVectorWeekday > 0 && intVectorWeekday <= days_mmax) {
                    if (intVectorWeekday < current_Monthday) {
                        next = intVectorWeekday;
                        break;
                    }
                }
            }
        }

        if (trigger_in_getNext == "sunrise") {
            if (current_time >= sunriseTime + offset_in_getNext) {

                if (next < current_Monthday) {
                    structnext_time.month = current_Month + period_offset + period_monthdays;
                } else {
                    structnext_time.month = current_Month + period_monthdays;
                }

                if (next == current_Monthday) {
                    for (int intVectorWeekday : intVectorWeekdays) {
                        if (intVectorWeekday > 0 && intVectorWeekday <= days_mmax) {
                            if (intVectorWeekday > next) {
                                next = intVectorWeekday;
                                break;
                            }
                        }
                    }
                }

                if (next == current_Monthday) {
                    for (int intVectorWeekday : intVectorWeekdays) {
                        if (intVectorWeekday > 0 && intVectorWeekday <= days_mmax) {
                            if (intVectorWeekday < next) {
                                next = intVectorWeekday;
                                break;
                            }
                        }
                    }
                }

                if (intVectorWeekdays.at(intVectorWeekdays.size() - 1) == current_Monthday) {
                    structnext_time.month = current_Month + period_offset + period_monthdays;
                }

                if (structnext_time.month > 12) {
                    structnext_time.month = structnext_time.month - 12;
                    structnext_time.year = structnext_time.year + 1;
                } else {
                    structnext_time.year = year_in_getNext;
                }

                structnext_time.time = sunriseTime + offset_in_getNext;
                structnext_time.day = next;
                return structnext_time;
            }
            if (sunriseTime + offset_in_getNext > current_time) {

                structnext_time.time = sunriseTime + offset_in_getNext;
                structnext_time.day = next;

                if (next < current_Monthday) {
                    structnext_time.month = current_Month + period_offset + period_monthdays;
                } else {
                    structnext_time.month = current_Month + period_monthdays;
                }

                if (structnext_time.month > 12) {
                    structnext_time.month = structnext_time.month - 12;
                    structnext_time.year = structnext_time.year + 1;
                } else {
                    structnext_time.year = year_in_getNext;
                }

                return structnext_time;

            }
        }
        if (trigger_in_getNext == "sunset") {

            if (current_time >= sunsetTime + offset_in_getNext) {

                if (next < current_Monthday) {
                    structnext_time.month = current_Month + period_offset + period_monthdays;
                } else {
                    structnext_time.month = current_Month + period_monthdays;
                }

                if (next == current_Monthday) {
                    for (int intVectorWeekday : intVectorWeekdays) {
                        if (intVectorWeekday > 0 && intVectorWeekday <= days_mmax) {
                            if (intVectorWeekday > next) {
                                next = intVectorWeekday;
                                break;
                            }
                        }
                    }
                }

                if (next == current_Monthday) {
                    for (int intVectorWeekday : intVectorWeekdays) {
                        if (intVectorWeekday > 0 && intVectorWeekday <= days_mmax) {
                            if (intVectorWeekday < next) {
                                next = intVectorWeekday;
                                break;
                            }
                        }
                    }
                }

                if (intVectorWeekdays.at(intVectorWeekdays.size() - 1) == current_Monthday) {
                    structnext_time.month = current_Month + period_offset + period_monthdays;
                }

                if (structnext_time.month > 12) {
                    structnext_time.month = structnext_time.month - 12;
                    structnext_time.year = structnext_time.year + 1;
                } else {
                    structnext_time.year = year_in_getNext;
                }

                structnext_time.time = sunsetTime + offset_in_getNext;
                structnext_time.day = next;
                return structnext_time;
            }
            if (sunsetTime + offset_in_getNext > current_time) {

                structnext_time.time = sunsetTime + offset_in_getNext;
                structnext_time.day = next;

                if (next < current_Monthday) {
                    structnext_time.month = current_Month + period_offset + period_monthdays;
                } else {
                    structnext_time.month = current_Month + period_monthdays;
                }

                if (structnext_time.month > 12) {
                    structnext_time.month = structnext_time.month - 12;
                    structnext_time.year = structnext_time.year + 1;
                } else {
                    structnext_time.year = year_in_getNext;
                }

                return structnext_time;
            }

        }
        if (trigger_in_getNext == "timepoint") {
            if (current_time >= day_start_in_getNext + timepoint_min_in_getNext) {

                if (next < current_Monthday) {
                    structnext_time.month = current_Month + period_offset + period_monthdays;
                } else {
                    structnext_time.month = current_Month + period_monthdays;
                }

                if (next == current_Monthday) {
                    for (int intVectorWeekday : intVectorWeekdays) {
                        if (intVectorWeekday > 0 && intVectorWeekday <= days_mmax) {
                            if (intVectorWeekday > next) {
                                next = intVectorWeekday;
                                break;
                            }
                        }
                    }
                }

                if (next == current_Monthday) {
                    for (int intVectorWeekday : intVectorWeekdays) {
                        if (intVectorWeekday > 0 && intVectorWeekday <= days_mmax) {
                            if (intVectorWeekday < next) {
                                next = intVectorWeekday;
                                break;
                            }
                        }
                    }
                }

                if (intVectorWeekdays.at(intVectorWeekdays.size() - 1) == current_Monthday) {
                    structnext_time.month = current_Month + period_offset + period_monthdays;
                }

                if (structnext_time.month > 12) {
                    structnext_time.month = structnext_time.month - 12;
                    structnext_time.year = structnext_time.year + 1;
                } else {
                    structnext_time.year = year_in_getNext;
                }

                structnext_time.time = day_start_in_getNext + timepoint_min_in_getNext;
                structnext_time.day = next;
                return structnext_time;
            }
            if (day_start_in_getNext + timepoint_min_in_getNext > current_time) {

                structnext_time.time = day_start_in_getNext + timepoint_min_in_getNext;
                structnext_time.day = next;

                if (next < current_Monthday) {
                    structnext_time.month = current_Month + period_offset + period_monthdays;
                } else {
                    structnext_time.month = current_Month + period_monthdays;
                }

                if (structnext_time.month > 12) {
                    structnext_time.month = structnext_time.month - 12;
                    structnext_time.year = structnext_time.year + 1;
                } else {
                    structnext_time.year = year_in_getNext;
                }
                return structnext_time;
            }

        }
    }
    if (type_in_getNext == "yearly") {
        int next_day = 0;
        int next_month = 0;
        int current_Monthday = tm.tm_mday;
        int current_Month = tm.tm_mon;
        int max_months = 12;
        int period_offset = 0;

        if (period_in_getNext > 1) {
            period_offset = 0;
        } else {
            period_offset = 1;
        }

        int period_monthdays;

        if (period_in_getNext > 1) {
            period_monthdays = period_in_getNext - 1;
        } else {
            period_monthdays = 0;
        }
        std::vector<int> intVectorMonthdays = BoolVectorToIntVector(_days);
        std::vector<int> intVectorMonths = BoolVectorToIntVector(_months);

        for (int intVectorMonth : intVectorMonths) {
            if (intVectorMonth > 0 && intVectorMonth <= max_months) {
                if (intVectorMonth >= current_Month) {
                    next_month = intVectorMonth;
                    break;
                }
            }
        }

        if (next_month == 0) {
            for (int intVectorMonth : intVectorMonths) {
                if (intVectorMonth > 0 && intVectorMonth <= max_months) {
                    if (intVectorMonth < current_Month) {
                        next_month = intVectorMonth;
                        break;
                    }
                }
            }
        }

        for (int intVectorWeekday : intVectorMonthdays) {
            if (intVectorWeekday > 0 && intVectorWeekday <= days_mmax) {
                if (intVectorWeekday >= current_Monthday && next_month) {
                    next_day = intVectorWeekday;
                    break;
                }
            }
        }

        if (next_day == 0 || next_month != current_Month) {
            for (int intVectorWeekday : intVectorMonthdays) {
                if (intVectorWeekday > 0 && intVectorWeekday <= days_mmax) {
                    if (intVectorWeekday < current_Monthday) {
                        next_day = intVectorWeekday;
                        break;
                    }
                }
            }
        }

        if (trigger_in_getNext == "sunrise") {
            if (current_time >= sunriseTime + offset_in_getNext) {

                if (next_day == current_Monthday) {
                    for (int intVectorWeekday : intVectorMonthdays) {
                        if (intVectorWeekday > 0 && intVectorWeekday <= days_mmax) {
                            if (intVectorWeekday > next_day) {
                                next_day = intVectorWeekday;
                                break;
                            }
                        }
                    }
                }

                if (next_day == current_Monthday) {
                    for (int intVectorWeekday : intVectorMonthdays) {
                        if (intVectorWeekday > 0 && intVectorWeekday <= days_mmax) {
                            if (intVectorWeekday < next_day) {
                                next_day = intVectorWeekday;
                                break;
                            }
                        }
                    }
                }

                if (next_month == current_Month && next_day < current_Monthday) {
                    for (int intVectorMonth : intVectorMonths) {
                        if (intVectorMonth > 0 && intVectorMonth <= max_months) {
                            if (intVectorMonth > next_month) {
                                next_month = intVectorMonth;
                                break;
                            }
                        }
                    }
                }

                if (next_month == current_Month && next_day < current_Monthday) {
                    for (int intVectorMonth : intVectorMonths) {
                        if (intVectorMonth > 0 && intVectorMonth <= max_months) {
                            if (intVectorMonth < next_month) {
                                next_month = intVectorMonth;
                                break;
                            }
                        }
                    }
                }
                if (next_day < current_Monthday) {
                    structnext_time.month = next_month + period_offset + period_monthdays;
                } else {
                    structnext_time.month = next_month + period_monthdays;
                }

                if (next_month < current_Month) {

                    structnext_time.month = next_month;
                    structnext_time.year = year_in_getNext + period_offset + period_monthdays;
                }

                if (next_month > current_Month) {
                    structnext_time.year = year_in_getNext + period_monthdays;
                    structnext_time.month = next_month;
                }

                if (next_day < current_Monthday && next_month == current_Month) {
                    structnext_time.year = structnext_time.year + period_offset;
                }

                if (intVectorMonthdays.at(intVectorMonthdays.size() - 1) == current_Monthday) {
                    structnext_time.year = year_in_getNext + period_monthdays + period_offset;
                }

                if (structnext_time.month > 12) {
                    structnext_time.month = structnext_time.month - 12;
                    structnext_time.year = structnext_time.year + 1;
                }

                structnext_time.time = sunriseTime + offset_in_getNext;
                structnext_time.day = next_day;
                return structnext_time;
            }
            if (sunriseTime + offset_in_getNext > current_time) {

                structnext_time.time = sunriseTime + offset_in_getNext;
                structnext_time.day = next_day;
                structnext_time.month = next_month;

                if (next_day < current_Monthday) {
                    structnext_time.month = next_month;
                } else {
                    structnext_time.month = next_month + period_monthdays;
                }
                if (next_month < current_Month) {

                    structnext_time.month = next_month;
                    structnext_time.year = year_in_getNext + period_offset + period_monthdays;
                } else {
                    structnext_time.year = year_in_getNext + period_monthdays;
                    structnext_time.month = next_month;
                }

                if (structnext_time.month > 12) {
                    structnext_time.month = structnext_time.month - 12;
                    structnext_time.year = structnext_time.year + 1;
                }
                if (next_day < current_Monthday && next_month == current_Month) {
                    structnext_time.year = structnext_time.year + period_offset + period_monthdays;
                }

                return structnext_time;

            }
        }
        if (trigger_in_getNext == "sunset") {

            if (current_time >= sunsetTime + offset_in_getNext) {

                if (next_day == current_Monthday) {
                    for (int intVectorWeekday : intVectorMonthdays) {
                        if (intVectorWeekday > 0 && intVectorWeekday <= days_mmax) {
                            if (intVectorWeekday > next_day) {
                                next_day = intVectorWeekday;
                                break;
                            }
                        }
                    }
                }

                if (next_day == current_Monthday) {
                    for (int intVectorWeekday : intVectorMonthdays) {
                        if (intVectorWeekday > 0 && intVectorWeekday <= days_mmax) {
                            if (intVectorWeekday < next_day) {
                                next_day = intVectorWeekday;
                                break;
                            }
                        }
                    }
                }

                if (next_month == current_Month && next_day < current_Monthday) {
                    for (int intVectorMonth : intVectorMonths) {
                        if (intVectorMonth > 0 && intVectorMonth <= max_months) {
                            if (intVectorMonth > next_month) {
                                next_month = intVectorMonth;
                                break;
                            }
                        }
                    }
                }

                if (next_month == current_Month && next_day < current_Monthday) {
                    for (int intVectorMonth : intVectorMonths) {
                        if (intVectorMonth > 0 && intVectorMonth <= max_months) {
                            if (intVectorMonth < next_month) {
                                next_month = intVectorMonth;
                                break;
                            }
                        }
                    }
                }
                if (next_day < current_Monthday) {
                    structnext_time.month = next_month + period_offset + period_monthdays;
                } else {
                    structnext_time.month = next_month + period_monthdays;
                }

                if (next_month < current_Month) {

                    structnext_time.month = next_month;
                    structnext_time.year = year_in_getNext + period_offset + period_monthdays;
                } else {
                    structnext_time.year = year_in_getNext + period_monthdays;
                    structnext_time.month = next_month;
                }

                if (intVectorMonthdays.at(intVectorMonthdays.size() - 1) == current_Monthday) {
                    structnext_time.year = year_in_getNext + period_monthdays + period_offset;
                }

                if (structnext_time.month > 12) {
                    structnext_time.month = structnext_time.month - 12;
                    structnext_time.year = structnext_time.year + 1;
                }

                structnext_time.time = sunsetTime + offset_in_getNext;
                structnext_time.day = next_day;
                return structnext_time;

            }
            if (sunsetTime + offset_in_getNext > current_time) {

                structnext_time.time = sunsetTime + offset_in_getNext;
                structnext_time.day = next_day;
                structnext_time.month = next_month;

                if (next_day < current_Monthday) {
                    structnext_time.month = next_month;
                } else {
                    structnext_time.month = next_month + period_monthdays;
                }
                if (next_month < current_Month) {

                    structnext_time.month = next_month;
                    structnext_time.year = year_in_getNext + period_offset + period_monthdays;
                } else {
                    structnext_time.year = year_in_getNext + period_monthdays;
                    structnext_time.month = next_month;
                }

                if (structnext_time.month > 12) {
                    structnext_time.month = structnext_time.month - 12;
                    structnext_time.year = structnext_time.year + 1;
                }
                if (next_day < current_Monthday && next_month == current_Month) {
                    structnext_time.year = structnext_time.year + period_offset + period_monthdays;
                }

                return structnext_time;
            }

        }
        if (trigger_in_getNext == "timepoint") {
            if (current_time >= day_start_in_getNext + timepoint_min_in_getNext) {

                if (next_day == current_Monthday) {
                    for (int intVectorWeekday : intVectorMonthdays) {
                        if (intVectorWeekday > 0 && intVectorWeekday <= days_mmax) {
                            if (intVectorWeekday > next_day) {
                                next_day = intVectorWeekday;
                                break;
                            }
                        }
                    }
                }

                if (next_day == current_Monthday) {
                    for (int intVectorWeekday : intVectorMonthdays) {
                        if (intVectorWeekday > 0 && intVectorWeekday <= days_mmax) {
                            if (intVectorWeekday < next_day) {
                                next_day = intVectorWeekday;
                                break;
                            }
                        }
                    }
                }

                if (next_month == current_Month && next_day < current_Monthday) {
                    for (int intVectorMonth : intVectorMonths) {
                        if (intVectorMonth > 0 && intVectorMonth <= max_months) {
                            if (intVectorMonth > next_month) {
                                next_month = intVectorMonth;
                                break;
                            }
                        }
                    }
                }

                if (next_month == current_Month && next_day < current_Monthday) {
                    for (int intVectorMonth : intVectorMonths) {
                        if (intVectorMonth > 0 && intVectorMonth <= max_months) {
                            if (intVectorMonth < next_month) {
                                next_month = intVectorMonth;
                                break;
                            }
                        }
                    }
                }
                if (next_day < current_Monthday) {
                    structnext_time.month = next_month + period_offset + period_monthdays;
                } else {
                    structnext_time.month = next_month + period_monthdays;
                }

                if (next_month < current_Month) {

                    structnext_time.month = next_month;
                    structnext_time.year = year_in_getNext + period_offset + period_monthdays;
                } else {
                    structnext_time.year = year_in_getNext + period_monthdays;
                    structnext_time.month = next_month;
                }

                if (intVectorMonthdays.at(intVectorMonthdays.size() - 1) == current_Monthday) {
                    structnext_time.year = year_in_getNext + period_monthdays + period_offset;
                }

                if (structnext_time.month > 12) {
                    structnext_time.month = structnext_time.month - 12;
                    structnext_time.year = structnext_time.year + 1;
                }

                structnext_time.time = day_start_in_getNext + timepoint_min_in_getNext;
                structnext_time.day = next_day;
                return structnext_time;
            }

            if (day_start_in_getNext + timepoint_min_in_getNext > current_time) {

                structnext_time.time = day_start_in_getNext + timepoint_min_in_getNext;
                structnext_time.day = next_day;
                structnext_time.month = next_month;

                if (next_day < current_Monthday) {
                    structnext_time.month = next_month;
                } else {
                    structnext_time.month = next_month + period_monthdays;
                }
                if (next_month < current_Month) {

                    structnext_time.month = next_month;
                    structnext_time.year = year_in_getNext + period_offset + period_monthdays;
                } else {
                    structnext_time.year = year_in_getNext + period_monthdays;
                    structnext_time.month = next_month;
                }

                if (structnext_time.month > 12) {
                    structnext_time.month = structnext_time.month - 12;
                    structnext_time.year = structnext_time.year + 1;
                }
                if (next_day < current_Monthday && next_month == current_Month) {
                    structnext_time.year = structnext_time.year + period_offset + period_monthdays;
                }

                return structnext_time;
            }

        }
    }
}

int MyNode::SearchForHigherOrEqualNumber(std::vector<int> days, int day_max, int number, int valueForNoResult) {
    int i;
    for (int day : days) {
        if (day > 0 && day <= day_max) {
            if (day >= number) {
                i = day;
                return i;
            }
        }
    }
    i = valueForNoResult;
    return i;
}

int MyNode::SearchForSmallerNumber(std::vector<int> days, int day_max, int number) {
    int i;
    for (int day : days) {
        if (day > 0 && day <= day_max) {
            if (day < number) {
                i = day;
                return i;
            }
        }
    }
    if (i == 0) {
        i = number;
    }

    return i;
}

int MyNode::SearchForHigherNumber(std::vector<int> days, int day_max, int number) {
    int i;
    for (int day : days) {
        if (day > 0 && day <= day_max) {
            if (day > number) {
                i = day;
                return i;
            }
        }
    }
    i = number;
    return i;
}

int MyNode::GetDaysMax() {
    std::tm tm{};
    _sunTime.getTimeStruct(tm);
    int month = tm.tm_mon + 1;
    int year = (tm.tm_year - 100) + 2000;
    bool gap_year;
    int days_mmax = 0;

    if ((year % 100 != 0 && year % 4 == 0) || year % 400 == 0) {
        gap_year = true;
    } else {
        gap_year = false;
    }
    if ((month % 2) == 1) {
        days_mmax = 31;
    } else {
        days_mmax = 30;
        if (gap_year && month == 2) {
            days_mmax = 29;
        }
        if (!gap_year && month == 2) {
            days_mmax = 28;
        }
    }
    return days_mmax;
}

std::vector<int32_t> MyNode::BoolVectorToIntVector(std::vector<bool> boolVal) {
    std::vector<int32_t> intNumbers;
    intNumbers.resize(boolVal.size() + 1);
    int j = 1;
    int c = 1;

    for(int i = 0; i < boolVal.size(); i++){
        if (boolVal.at(i)){
            c++;
        }
    }

    for(int i = 0; i < boolVal.size(); i++){
        if (boolVal.at(i)){
            intNumbers.at(j) = i + 1;
            j++;
        }
    }
    intNumbers.resize(c);
    return intNumbers;
}


void MyNode::printNext(NextTime next) {

    int64_t day_next = next.day;
    int64_t month_next = next.month;
    int64_t year_next = next.year;

    next.time = next.time / 1000;

    next.time = next.time % 86400;;
    int32_t hours_next = next.time / 3600;

    next.time = next.time % 3600;

    int32_t minutes_next = next.time / 60;

    int32_t seconds_next = next.time % 60;

    std::ostringstream timeStream;
    Flows::PVariable status = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);

    timeStream << std::setw(2) << std::setfill('0') << day_next << "." << std::setw(2) << month_next << "."
               << std::setw(2) << year_next << " " << std::setw(2) << hours_next << ':' << std::setw(2) << minutes_next << ':'
               << std::setw(2) << seconds_next;

    status->structValue->emplace("text", std::make_shared<Flows::Variable>("Next: " + timeStream.str()));
    nodeEvent("statusBottom/" + _id, status, true);

}

void MyNode::timer() {

    int64_t currentTime = _sunTime.getLocalTime();
    int64_t lastTime = currentTime;
    std::tm tm{};
    _sunTime.getTimeStruct(tm);
    auto next_time = getNext();
    printNext(next_time);
    bool update = false;
    int64_t time = getTime(SunTime().getLocalTime(), "sunrise", "suntime", 0);
    std::tm timeStruct{};

    _sunTime.getTimeStruct(timeStruct);

    int64_t time_next = next_time.time;
    time_next = time_next / 1000;
    time_next = time_next % 86400;
    int day_next = next_time.day;
    int month_next = next_time.month;
    int year_next = next_time.year;

    //TODO add Workday/Weekend check if type == daily
    while (!_stopThread) {
        try {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            if (_stopThread) break;

            currentTime = _sunTime.getLocalTime();
            currentTime = currentTime / 1000;
            currentTime = currentTime % 86400;
            int current_day = tm.tm_mday;
            int current_month = tm.tm_mon + 1;
            int current_year = tm.tm_year % 100 + 2000;

            time_next = next_time.time;
            time_next = time_next / 1000;
            time_next = time_next % 86400;
            day_next = next_time.day;
            month_next = next_time.month;
            year_next = next_time.year;

            if (currentTime == time_next && current_day == day_next && current_month == month_next && current_year == year_next) {

                update = true;
                _lastTime = time_next;
                setNodeData("lastOnTime", std::make_shared<Flows::Variable>(_lastTime));
                Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                message->structValue->emplace("payload", std::make_shared<Flows::Variable>(true));
                output(0, message);

            }

            if (update || _forceUpdate || currentTime < lastTime) //New hour? Recalc in case of time changes or summer/winter time
            {
                update = false;
                _forceUpdate = false;
                next_time = getNext();

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