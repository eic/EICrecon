// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong

#pragma once

#include <Acts/Utilities/Logger.hpp>
#include <GaudiKernel/ServiceHandle.h>
#include <GaudiKernel/IMessageSvc.h>
#include <GaudiKernel/MsgStream.h>

/** Filter
 *
 * \ingroup base
 */
class GaudiFilterPolicy : public Acts::Logging::OutputFilterPolicy {
public:
  GaudiFilterPolicy(IMessageSvc* owner) : m_owner(owner) {}

  bool doPrint(const Acts::Logging::Level& lvl) const {

    MSG::Level l = MSG::VERBOSE;
    switch (lvl) {
    case Acts::Logging::VERBOSE:
      l = MSG::VERBOSE;
      break;
    case Acts::Logging::DEBUG:
      l = MSG::DEBUG;
      break;
    case Acts::Logging::INFO:
      l = MSG::INFO;
      break;
    case Acts::Logging::WARNING:
      l = MSG::WARNING;
      break;
    case Acts::Logging::ERROR:
      l = MSG::ERROR;
      break;
    case Acts::Logging::FATAL:
      l = MSG::FATAL;
      break;
     case Acts::Logging::MAX:
      l = MSG::VERBOSE;
      break;
    }

    return l >= m_owner->outputLevel();
  }

private:
  IMessageSvc* m_owner;
};

class GaudiPrintPolicy : public Acts::Logging::OutputPrintPolicy {
public:
  GaudiPrintPolicy(IMessageSvc* owner) : m_messenger(owner) {}

  void setName(std::string name) {
    m_name = name;

  }

  void flush(const Acts::Logging::Level& lvl, const std::string& input) {
    MSG::Level l = MSG::VERBOSE;
    switch (lvl) {
    case Acts::Logging::VERBOSE:
      l = MSG::VERBOSE;
      break;
    case Acts::Logging::DEBUG:
      l = MSG::DEBUG;
      break;
    case Acts::Logging::INFO:
      l = MSG::INFO;
      break;
    case Acts::Logging::WARNING:
      l = MSG::WARNING;
      break;
    case Acts::Logging::ERROR:
      l = MSG::ERROR;
      break;
    case Acts::Logging::FATAL:
      l = MSG::FATAL;
      break;
     case Acts::Logging::MAX:
      l = MSG::VERBOSE;
      break;
    }

    m_messenger << l << m_name << "\t" << input << endmsg;
  }

private:
  IMessageSvc* m_owner;
  MsgStream m_messenger;
  std::string m_name;
};
