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
#include "math.h"

namespace scale {

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

        settingsIterator = info->info->structValue->find("value");
        if (settingsIterator != info->info->structValue->end())
            _value = Flows::Math::getDouble(settingsIterator->second->stringValue);

        settingsIterator = info->info->structValue->find("valueMin");
        if (settingsIterator != info->info->structValue->end())
            _valueMin = Flows::Math::getDouble(settingsIterator->second->stringValue);

        settingsIterator = info->info->structValue->find("valueMax");
        if (settingsIterator != info->info->structValue->end())
            _valueMax = Flows::Math::getDouble(settingsIterator->second->stringValue);

        settingsIterator = info->info->structValue->find("scaleMin");
        if (settingsIterator != info->info->structValue->end())
            _scaleMin = Flows::Math::getDouble(settingsIterator->second->stringValue);

        settingsIterator = info->info->structValue->find("scaleMax");
        if (settingsIterator != info->info->structValue->end())
            _scaleMax = Flows::Math::getDouble(settingsIterator->second->stringValue);

        settingsIterator = info->info->structValue->find("intOrDouble");
        if (settingsIterator != info->info->structValue->end())
            _intOrDouble = settingsIterator->second->booleanValue;

        settingsIterator = info->info->structValue->find("manualInput");
        if (settingsIterator != info->info->structValue->end())
            _manualInput = settingsIterator->second->booleanValue;

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

int32_t MyNode::scaleInt(int32_t value, int32_t valueMin, int32_t valueMax, int32_t scaleMin, int32_t scaleMax)
{
    double vPerc = ((double)(value - valueMin)) / (valueMax - valueMin);
    double bigSpan = vPerc * (scaleMax - scaleMin);

    return std::lround(scaleMin + bigSpan);
}

double MyNode::scaleDouble(double value, double valueMin, double valueMax, double scaleMin, double scaleMax)
{
    double vPerc = (value - valueMin) / (valueMax - valueMin);
    double bigSpan = vPerc * (scaleMax - scaleMin);

    return scaleMin + bigSpan;
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

void MyNode::timer() {


    if (_intOrDouble) {
        double erg = scaleDouble(_value, _valueMin, _valueMax, _scaleMin, _scaleMax);

        Flows::PVariable status = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
        status->structValue->emplace("text", std::make_shared<Flows::Variable>("Ergebnis: " + std::to_string(erg)));
        nodeEvent("statusBottom/" + _id, status, true);
        _lastValue = _value;
        _out->printError("erg11 " + std::to_string(erg));

        setNodeData("value", std::make_shared<Flows::Variable>(erg));
        Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
        message->structValue->emplace("payload", std::make_shared<Flows::Variable>(erg));
        output(0, message);
    }else{
        int erg = scaleInt(_value, _valueMin, _valueMax, _scaleMin, _scaleMax);

        Flows::PVariable status = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
        status->structValue->emplace("text", std::make_shared<Flows::Variable>("Ergebnis: " + std::to_string(erg)));
        nodeEvent("statusBottom/" + _id, status, true);
        _lastValue = _value;
        _out->printError("erg12 " + std::to_string(erg));

        setNodeData("value", std::make_shared<Flows::Variable>(erg));
        Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
        message->structValue->emplace("payload", std::make_shared<Flows::Variable>(erg));
        output(0, message);
    }

    while (!_stopThread) {
        try {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            if (_stopThread) {
                break;
            }

            if (_lastValue != _value) {
                if (_intOrDouble) {
                    double erg = scaleDouble(_value, _valueMin, _valueMax, _scaleMin, _scaleMax);

                    Flows::PVariable status = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                    status->structValue->emplace("text", std::make_shared<Flows::Variable>("Ergebnis: " + std::to_string(erg)));
                    nodeEvent("statusBottom/" + _id, status, true);
                    _lastValue = _value;
                    _out->printError("erg21 " + std::to_string(erg));

                    setNodeData("value", std::make_shared<Flows::Variable>(erg));
                    Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                    message->structValue->emplace("payload", std::make_shared<Flows::Variable>(erg));
                    output(0, message);

                } else {
                    int erg = scaleInt(_value, _valueMin, _valueMax, _scaleMin, _scaleMax);

                    Flows::PVariable status = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                    status->structValue->emplace("text", std::make_shared<Flows::Variable>("Ergebnis: " + std::to_string(erg)));
                    nodeEvent("statusBottom/" + _id, status, true);
                    _lastValue = _value;
                    _out->printError("erg22 " + std::to_string(erg));

                    setNodeData("value", std::make_shared<Flows::Variable>(erg));
                    Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                    message->structValue->emplace("payload", std::make_shared<Flows::Variable>(erg));
                    output(0, message);
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
        if (index == 1 && !_manualInput)
        {
            _value = message->structValue->at("payload")->floatValue;
            setNodeData("value", std::make_shared<Flows::Variable>(_value));

        }
        if (index == 2 && !_manualInput)
        {
            _valueMin = message->structValue->at("payload")->floatValue;
            setNodeData("valueMin", std::make_shared<Flows::Variable>(_valueMin));

        }
        if (index == 3 && !_manualInput)
        {
            _valueMax = message->structValue->at("payload")->floatValue;
            setNodeData("valueMax", std::make_shared<Flows::Variable>(_valueMax));

        }
        if (index == 4 && !_manualInput)
        {
            _scaleMin = message->structValue->at("payload")->floatValue;
            setNodeData("scaleMin", std::make_shared<Flows::Variable>(_scaleMin));

        }
        if (index == 5 && !_manualInput)
        {
            _scaleMax = message->structValue->at("payload")->floatValue;
            setNodeData("scaleMax", std::make_shared<Flows::Variable>(_scaleMax));

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