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

        settingsIterator = info->info->structValue->find("days");
        if (settingsIterator != info->info->structValue->end())
            _days = settingsIterator->second->stringValue;

        settingsIterator = info->info->structValue->find("weekdays");
        if (settingsIterator != info->info->structValue->end())
            _weekdays = settingsIterator->second->stringValue;

        settingsIterator = info->info->structValue->find("daysnumber");
        if (settingsIterator != info->info->structValue->end())
            _daysNumber = Flows::Math::getNumber(settingsIterator->second->stringValue);

        settingsIterator = info->info->structValue->find("month");
        if (settingsIterator != info->info->structValue->end())
            _months = Flows::Math::getNumber(settingsIterator->second->stringValue);

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

std::array<int64_t, 100> MyNode::ConvertStrtoArr(std::string timepoint) {
    if (timepoint.length() > 0) {

        int64_t j = 0, i;
        std::array<int64_t, 100> arr = {0};
        for (i = 0; timepoint[i] != '\0'; i++) {

            if (timepoint[i] == ' ')
                continue;
            if (timepoint[i] == ',') {
                j++;
            } else {
                arr[j] = arr[j] * 10 + (timepoint[i] - 48);
            }
        }

        return arr;

    }
}

MyNode::NextTime MyNode::getNext() {
    struct NextTime structnext_time;
    std::string type_in_getNext = _type;
    std::string trigger_in_getNext = _trigger;
    int64_t offset_in_getNext = _onOffset * 1000 * 60;
    int64_t period_in_getNext = _period;
    std::string days_in_getNext = _days;
    int64_t days_number_in_getNext = _daysNumber;
    int64_t months_in_getNext;
    std::string weekdays_in_getNext = _weekdays;
    int64_t current_time = _sunTime.getLocalTime();
    int64_t inputTime = current_time - 86400000;
    int64_t sunriseTime = getSunTime(current_time, "sunrise");
    int64_t sunsetTime = getSunTime(current_time, "sunset");
    int64_t timepoint_min_in_getNext = _timepoint * 1000 * 60;
    std::tm tm{};
    _sunTime.getTimeStruct(tm);
    int month_in_getNext = tm.tm_year / 100;
    int year_in_getNext = (tm.tm_year - (month_in_getNext * 100)) + 2000;
    int64_t day_start_in_getNext = current_time - (current_time % 86400000);
    int weekday_index = 0;
    int weekday_offset = 0;
    bool gap_year;
    int days_mmax = 0;

    if ((year_in_getNext % 100 != 0 && year_in_getNext % 4 == 0) || year_in_getNext % 400 == 0) {
        gap_year = true;
    } else {
        gap_year = false;
    }

    tm.tm_mon = tm.tm_mon + 1; //Korrektur da Jan=0,Feb=1,...


    if (tm.tm_wday == 0) {
        tm.tm_wday = 7; //Korrektur da Son=0, Mon=1,...
    }

    int current_weekday = tm.tm_wday;

    if (((tm.tm_mon) % 2) == 1) {
        days_mmax = 31;
    } else {
        days_mmax = 30;
        if (gap_year && tm.tm_mon == 2) {
            days_mmax = 29;
        }
        if (!gap_year && tm.tm_mon == 2) {
            days_mmax = 28;
        }
    }

    if (period_in_getNext == 1) {
        period_in_getNext = 0;
    } else {
        period_in_getNext = period_in_getNext - 1;
    }

    if (type_in_getNext == "daily") {

        if (days_in_getNext == "weekend") {
            if (tm.tm_wday < 6) {
                weekday_offset = 6 - tm.tm_wday;
                period_in_getNext = 0;
            }
        }
        if (days_in_getNext == "workday") {
            if (tm.tm_wday > 5) {
                weekday_offset = (tm.tm_wday - 8) * (-1);
                period_in_getNext = 0;
            }
        }

        if (trigger_in_getNext == "sunrise") {
            if (current_time >= sunriseTime + offset_in_getNext) {

                structnext_time.time = sunriseTime + offset_in_getNext + (weekday_offset * 86400000) + (period_in_getNext * 86400000) + 86400000;
                structnext_time.day = tm.tm_mday + weekday_offset + period_in_getNext + 1; // TODO in constante

                if (structnext_time.day > days_mmax) {
                    structnext_time.day = structnext_time.day - days_mmax;
                    structnext_time.month = month_in_getNext + 1;
                } else {
                    structnext_time.month = month_in_getNext;
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
                    structnext_time.month = month_in_getNext + 1;
                } else {
                    structnext_time.month = month_in_getNext;
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
                structnext_time.time = sunsetTime + offset_in_getNext + (weekday_offset * 86400000) + (period_in_getNext * 86400000) + 86400000;
                structnext_time.day = tm.tm_mday + weekday_offset + period_in_getNext + 1;

                if (structnext_time.day > days_mmax) {
                    structnext_time.day = structnext_time.day - days_mmax;
                    structnext_time.month = month_in_getNext + 1;
                } else {
                    structnext_time.month = month_in_getNext;
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
                    structnext_time.month = month_in_getNext + 1;
                } else {
                    structnext_time.month = month_in_getNext;
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

                structnext_time.time = day_start_in_getNext + timepoint_min_in_getNext + (weekday_offset * 86400000) + (period_in_getNext * 86400000) + 86400000;
                structnext_time.day = tm.tm_mday + weekday_offset + period_in_getNext + 1;

                if (structnext_time.day > days_mmax) {
                    structnext_time.day = structnext_time.day - days_mmax;
                    structnext_time.month = month_in_getNext + 1;
                } else {
                    structnext_time.month = month_in_getNext;
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
                    structnext_time.month = month_in_getNext + 1;
                } else {
                    structnext_time.month = month_in_getNext;
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
        std::string WeekdaysFiltered = stringFilter(weekdays_in_getNext, "','");
        _out->printError("WeekdaysFiltered " + WeekdaysFiltered);
        char weekdays_array[WeekdaysFiltered.length() + 1];
        int next = 0;
        int following_next = 0;
        strcpy(weekdays_array, WeekdaysFiltered.c_str());

        for (int i = 0; i < 7; i++) {
            if (weekdays_array[i] > 0 && weekdays_array[i] - 48 > 0 && weekdays_array[i] - 48 <= 7) {
                _out->printError("WeekdaysArray " +std::to_string(weekdays_array[i] - 48));
                if (weekdays_array[i] - 48 >= current_weekday) {
                    next = weekdays_array[i] - 48;
                    _out->printError("next " + std::to_string(next));
                    break;
                }
            }
        }
        for (int j = 0; j < 7; j++) {
            if (weekdays_array[j] > 0 && weekdays_array[j] - 48 > 0 && weekdays_array[j] - 48 <= 7) {
                if (weekdays_array[j] - 48 > next) {
                    following_next = weekdays_array[j] - 48;
                    break;
                }
            }
        }

        if (following_next == 0 && next != current_weekday) {
            _out->printError("following_next == 0");
            for (int j = 7; j >= 0; j--) {
                _out->printError("j2 " + std::to_string(j));
                    if (weekdays_array[j] - 48 < next) {
                        following_next = weekdays_array[j] - 48;
                        _out->printError("j3 " + std::to_string(j));
                        _out->printError("weekdays_array[j] " + std::to_string(weekdays_array[j] - 48));
                    }
            }
        }

        int offset_weekdays = next - current_weekday; //TODO anderer Name

        if (offset_weekdays < 0){
            offset_weekdays = 0;
        }

        int offset_next = following_next - next;
        if (offset_next < 0){
            offset_next = 0;
        }

        if (offset_next < 0) {
            offset_next = 0;
        }

        if (following_next < current_weekday && following_next > 0){
            offset_next = 7 - (current_weekday - following_next);
        }

        if (period_in_getNext != 0){
            offset_next = 0;
        }
        _out->printError("offset_weekdays " + std::to_string(offset_weekdays));
        _out->printError("offset_next " + std::to_string(offset_next));
        _out->printError("period_in_getNext " + std::to_string(period_in_getNext));
        _out->printError("follownext " + std::to_string(following_next));
        _out->printError("tmmday " + std::to_string(tm.tm_mday));
        _out->printError("next " + std::to_string(next));

        if (trigger_in_getNext == "sunrise") {
            if (current_time >= sunriseTime + offset_in_getNext) {
                if (following_next == 0 && current_weekday >= next){
                    offset_weekdays = 7;
                }

                structnext_time.time = sunriseTime + offset_next * 86400000 + (period_in_getNext * 86400000 * (period_in_getNext * 7));
                structnext_time.day = tm.tm_mday + offset_next + (period_in_getNext * 7) + offset_weekdays;

                if (structnext_time.day > days_mmax) {
                    structnext_time.day = structnext_time.day - days_mmax;
                    structnext_time.month = month_in_getNext + 1;
                } else {
                    structnext_time.month = month_in_getNext;
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

                structnext_time.time = sunriseTime + offset_in_getNext + (offset_weekdays * 86400000) + (period_in_getNext * 86400000 * (period_in_getNext * 7));
                structnext_time.day = tm.tm_mday + period_in_getNext * 7 + offset_weekdays;

                if (structnext_time.day > days_mmax) {
                    structnext_time.day = structnext_time.day - days_mmax;
                    structnext_time.month = month_in_getNext + 1;
                } else {
                    structnext_time.month = month_in_getNext;
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
                if (following_next == 0 && current_weekday >= next){
                    offset_weekdays = 7;
                }

                structnext_time.time = sunsetTime + offset_next * 86400000 + (period_in_getNext * 86400000 * (period_in_getNext * 7));
                structnext_time.day = tm.tm_mday + offset_next + (period_in_getNext * 7) + offset_weekdays;

                if (structnext_time.day > days_mmax) {
                    structnext_time.day = structnext_time.day - days_mmax;
                    structnext_time.month = month_in_getNext + 1;
                } else {
                    structnext_time.month = month_in_getNext;
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

                structnext_time.time = sunsetTime + offset_in_getNext + (offset_weekdays * 86400000) + (period_in_getNext * 86400000 * 7);
                structnext_time.day = tm.tm_mday + period_in_getNext * 7 + offset_weekdays;

                if (structnext_time.day > days_mmax) {
                    structnext_time.day = structnext_time.day - days_mmax;
                    structnext_time.month = month_in_getNext + 1;
                } else {
                    structnext_time.month = month_in_getNext;
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


                structnext_time.time = day_start_in_getNext + timepoint_min_in_getNext + (offset_next * 86400000) + (period_in_getNext * 86400000 * (period_in_getNext * 7));
                structnext_time.day = tm.tm_mday + offset_next + period_in_getNext * 7;

                if (structnext_time.day > days_mmax) {
                    structnext_time.day = structnext_time.day - days_mmax;
                    structnext_time.month = month_in_getNext + 1;
                } else {
                    structnext_time.month = month_in_getNext;
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
                _out->printError("current_weekday " + std::to_string(current_weekday));

                if (current_weekday == next){
                    period_in_getNext = 0;
                    offset_next = 0;
                }
                if (period_in_getNext != 0){
                    offset_next = 0;
                }

                structnext_time.time = day_start_in_getNext + timepoint_min_in_getNext + (offset_next * 86400000) + (period_in_getNext * 86400000 * 7);
                structnext_time.day = tm.tm_mday + offset_next + period_in_getNext * 7;


                if (structnext_time.day > days_mmax) {
                    structnext_time.day = structnext_time.day - days_mmax;
                    structnext_time.month = month_in_getNext + 1;
                } else {
                    structnext_time.month = month_in_getNext;
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
    if (type_in_getNext == "monthly") {
        if (trigger_in_getNext == "sunrise") {

        }
        if (trigger_in_getNext == "sunset") {

        }
        if (trigger_in_getNext == "timepoint") {

        }

    }
    if (type_in_getNext == "yearly") {
        if (trigger_in_getNext == "sunrise") {

        }
        if (trigger_in_getNext == "sunset") {

        }
        if (trigger_in_getNext == "timepoint") {

        }

    }

}

std::string MyNode::getDateString(int64_t time) {
    const char timeFormat[] = "%x";
    std::time_t t = 0;
    if (time > 0) {
        t = std::time_t(time / 1000);
    } else {
        const auto timePoint = std::chrono::system_clock::now();
        t = std::chrono::system_clock::to_time_t(timePoint);
    }
    char timeString[50];
    std::tm localTime{};
    localtime_r(&t, &localTime);
    strftime(&timeString[0], 50, &timeFormat[0], &localTime);
    std::ostringstream timeStream;
    timeStream << timeString;
    return timeStream.str();
}

std::string MyNode::stringFilter(const std::string &to, const std::string &remove) {
    std::string final;
    for (std::string::const_iterator it = to.begin(); it != to.end(); ++it) {
        if (remove.find(*it) == std::string::npos) {
            final += *it;
        }
    }
    return final;
}

void MyNode::printNext(NextTime next) {

    int64_t day_next = next.day;
    int64_t month_next = next.month;
    int64_t year_next = next.year;

    next.time = next.time / 1000;

    next.time = next.time % 86400;
    ;
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
    int periodcounter = _period;
    _sunTime.getTimeStruct(timeStruct);


    //TODO add Workday/Weekend check if type == daily
    while (!_stopThread) {
        try {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            if (_stopThread) break;
            currentTime = _sunTime.getLocalTime();

            _lastTime = next_time.time;

            if (currentTime / 1000 == next_time.time / 1000) {
                if (period_check) {
                    update = true;
                    _lastTime = next_time.time;
                    setNodeData("lastOnTime", std::make_shared<Flows::Variable>(_lastTime));
                    Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                    message->structValue->emplace("payload", std::make_shared<Flows::Variable>(true));
                    output(0, message);
                    period_check = false;
                } else {
                    periodcounter--;
                    if (periodcounter == 0) {
                        period_check = true;
                    }
                }
            }

            if (update || _forceUpdate || currentTime % 3600000 < lastTime % 3600000) //New hour? Recalc in case of time changes or summer/winter time
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