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

#include "MyNode.h"
#include "../../timers/weekly-program/MyNode.h"

#include <homegear-base/Security/SecureVector.h>

namespace MyNode {

MyNode::MyNode(const std::string &path, const std::string &type, const std::atomic_bool *frontendConnected) : Flows::INode(path, type, frontendConnected) {
  _bl.reset(new BaseLib::SharedObjects(false));

  _localRpcMethods.emplace("send", std::bind(&MyNode::send, this, std::placeholders::_1));
  _localRpcMethods.emplace("registerNode", std::bind(&MyNode::registerNode, this, std::placeholders::_1));
}

MyNode::~MyNode() = default;

bool MyNode::init(const Flows::PNodeInfo &info) {
  try {
    _nodeInfo = info;
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
    std::string type;
    std::string address;
    std::string port;

    auto settingsIterator = _nodeInfo->info->structValue->find("sockettype");
    if (settingsIterator != _nodeInfo->info->structValue->end()) type = settingsIterator->second->stringValue;

    if (type == "server") {
      BaseLib::TcpSocket::TcpServerInfo serverInfo;
      serverInfo.packetReceivedCallback = std::bind(&MyNode::packetReceived, this, std::placeholders::_1, std::placeholders::_2);

      settingsIterator = _nodeInfo->info->structValue->find("listenaddress");
      if (settingsIterator != _nodeInfo->info->structValue->end()) address = settingsIterator->second->stringValue;

      if (!address.empty() && !BaseLib::Net::isIp(address)) address = BaseLib::Net::getMyIpAddress(address);
      else if (address.empty()) address = BaseLib::Net::getMyIpAddress();

      settingsIterator = _nodeInfo->info->structValue->find("listenport");
      if (settingsIterator != _nodeInfo->info->structValue->end()) port = settingsIterator->second->stringValue;

      settingsIterator = _nodeInfo->info->structValue->find("usetlsserver");
      if (settingsIterator != _nodeInfo->info->structValue->end()) serverInfo.useSsl = settingsIterator->second->booleanValue;

      if (serverInfo.useSsl) {
        std::string tlsNodeId;
        settingsIterator = _nodeInfo->info->structValue->find("tlsserver");
        if (settingsIterator != _nodeInfo->info->structValue->end()) tlsNodeId = settingsIterator->second->stringValue;

        if (!tlsNodeId.empty()) {
          BaseLib::TcpSocket::PCertificateInfo certificateInfo = std::make_shared<BaseLib::TcpSocket::CertificateInfo>();
          certificateInfo->caFile = getConfigParameter(tlsNodeId, "ca")->stringValue;
          certificateInfo->caData = getConfigParameter(tlsNodeId, "cadata.password")->stringValue;
          certificateInfo->certFile = getConfigParameter(tlsNodeId, "cert")->stringValue;
          certificateInfo->certData = getConfigParameter(tlsNodeId, "certdata.password")->stringValue;
          certificateInfo->keyFile = getConfigParameter(tlsNodeId, "key")->stringValue;
          auto keyData = getConfigParameter(tlsNodeId, "keydata.password")->stringValue;
          auto secureKeyData = std::make_shared<BaseLib::Security::SecureVector<uint8_t>>();
          secureKeyData->insert(secureKeyData->end(), keyData.begin(), keyData.end());
          certificateInfo->keyData = secureKeyData;
          serverInfo.certificates.emplace("*", certificateInfo);
          serverInfo.dhParamData = getConfigParameter(tlsNodeId, "dhdata.password")->stringValue;
          serverInfo.dhParamFile = getConfigParameter(tlsNodeId, "dh")->stringValue;
          serverInfo.requireClientCert = getConfigParameter(tlsNodeId, "clientauth")->booleanValue;
        }
      }

      _socket = std::make_shared<BaseLib::TcpSocket>(_bl.get(), serverInfo);

      try {
        std::string listenAddress;
        _socket->startServer(address, port, listenAddress);
        _out->printInfo("Info: Server is now listening on address " + listenAddress + ".");
      }
      catch (BaseLib::Exception &ex) {
        _out->printError("Error starting server: " + std::string(ex.what()));
        return false;
      }
    } else if (type == "client") {
      bool useTls = false;

      settingsIterator = _nodeInfo->info->structValue->find("address");
      if (settingsIterator != _nodeInfo->info->structValue->end()) address = settingsIterator->second->stringValue;

      settingsIterator = _nodeInfo->info->structValue->find("port");
      if (settingsIterator != _nodeInfo->info->structValue->end()) port = settingsIterator->second->stringValue;

      settingsIterator = _nodeInfo->info->structValue->find("usetlsclient");
      if (settingsIterator != _nodeInfo->info->structValue->end()) useTls = settingsIterator->second->booleanValue;

      if (useTls) {
        std::string tlsNodeId;
        settingsIterator = _nodeInfo->info->structValue->find("tlsclient");
        if (settingsIterator != _nodeInfo->info->structValue->end()) tlsNodeId = settingsIterator->second->stringValue;

        if (!tlsNodeId.empty()) {
          BaseLib::TcpSocket::PCertificateInfo certificateInfo = std::make_shared<BaseLib::TcpSocket::CertificateInfo>();
          certificateInfo->caFile = getConfigParameter(tlsNodeId, "ca")->stringValue;
          certificateInfo->caData = getConfigParameter(tlsNodeId, "cadata.password")->stringValue;
          certificateInfo->certFile = getConfigParameter(tlsNodeId, "cert")->stringValue;
          certificateInfo->certData = getConfigParameter(tlsNodeId, "certdata.password")->stringValue;
          certificateInfo->keyFile = getConfigParameter(tlsNodeId, "key")->stringValue;
          auto keyData = getConfigParameter(tlsNodeId, "keydata.password")->stringValue;
          auto secureKeyData = std::make_shared<BaseLib::Security::SecureVector<uint8_t>>();
          secureKeyData->insert(secureKeyData->end(), keyData.begin(), keyData.end());
          certificateInfo->keyData = secureKeyData;

          _socket =
              std::make_shared<BaseLib::TcpSocket>(_bl.get(), address, port, useTls, true, certificateInfo->caFile, certificateInfo->caData, certificateInfo->certFile, certificateInfo->certData, certificateInfo->keyFile, certificateInfo->keyData);
        }
      } else {
        _socket = std::make_shared<BaseLib::TcpSocket>(_bl.get(), address, port);
      }

      if (_socket) {
        _socket->setConnectionRetries(1);
      }
    } else _out->printError("Error: Invalid socket type.");

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

void MyNode::stop() {
  try {
    //Todo: Stop server/client
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
    //Todo: Wait for server/client to stop
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

Flows::PVariable MyNode::getConfigParameterIncoming(const std::string &name) {
  try {
    auto settingsIterator = _nodeInfo->info->structValue->find(name);
    if (settingsIterator != _nodeInfo->info->structValue->end()) return settingsIterator->second;
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
  return std::make_shared<Flows::Variable>();
}

void MyNode::packetReceived(int32_t clientId, BaseLib::TcpSocket::TcpPacket &packet) {

}

//{{{ RPC methods
Flows::PVariable MyNode::send(const Flows::PArray &parameters) {
  try {
    if (parameters->size() != 4) return Flows::Variable::createError(-1, "Method expects exactly four parameters. " + std::to_string(parameters->size()) + " given.");
    if (parameters->at(0)->type != Flows::VariableType::tInteger && parameters->at(0)->type != Flows::VariableType::tInteger64) return Flows::Variable::createError(-1, "Parameter 1 is not of type integer.");
    if (parameters->at(1)->type != Flows::VariableType::tInteger && parameters->at(1)->type != Flows::VariableType::tInteger64) return Flows::Variable::createError(-1, "Parameter 2 is not of type integer.");
    if (parameters->at(2)->type != Flows::VariableType::tArray) return Flows::Variable::createError(-1, "Parameter 2 is not of type array.");
    if (parameters->at(3)->type != Flows::VariableType::tString) return Flows::Variable::createError(-1, "Parameter 4 is not of type string.");

    return std::make_shared<Flows::Variable>();
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
  return Flows::Variable::createError(-32500, "Unknown application error.");
}

Flows::PVariable MyNode::registerNode(const Flows::PArray &parameters) {
  try {
    if (parameters->size() != 3) return Flows::Variable::createError(-1, "Method expects exactly 3 parameters. " + std::to_string(parameters->size()) + " given.");
    if (parameters->at(0)->type != Flows::VariableType::tString) return Flows::Variable::createError(-1, "Parameter 1 is not of type string.");
    if (parameters->at(1)->type != Flows::VariableType::tString) return Flows::Variable::createError(-1, "Parameter 2 is not of type string.");
    if (parameters->at(2)->type != Flows::VariableType::tString) return Flows::Variable::createError(-1, "Parameter 3 is not of type string.");

    NodeInfo info;
    info.id = parameters->at(0)->stringValue;

    std::lock_guard<std::mutex> nodesGuard(_nodesMutex);
    _nodes[parameters->at(2)->stringValue].emplace(BaseLib::HelperFunctions::toUpper(parameters->at(1)->stringValue), std::move(info));

    return std::make_shared<Flows::Variable>();
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
  return Flows::Variable::createError(-32500, "Unknown application error.");
}
//}}}

}
