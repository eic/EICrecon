// Copyright 2024, EIC
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <JANA/JApplicationFwd.h>
#include <JANA/JEventSource.h>
#include <JANA/JEventSourceGeneratorT.h>
#include <spdlog/logger.h>
#include <memory>
#include <string>

#include "RCDAQFileReader.h"

/// JANA2 event source for rcdaq binary data files.
///
/// Reads ONCS-format rcdaq files and, for every DATA event, inserts one
/// RCDAQSubevent object per sub-event into the JEvent.  Downstream JANA
/// factories are responsible for decoding the raw sub-event data into EDM4hep
/// collections.
///
/// File format detection: CheckOpenable returns a positive score for files
/// whose names end in ".prdf", ".evt", or ".rcdaq".
class JEventSourceRCDAQ : public JEventSource {
public:
  JEventSourceRCDAQ(std::string resource_name, JApplication* app);

  ~JEventSourceRCDAQ() override = default;

  void Open() override;

  void Close() override;

  Result Emit(JEvent& event) override;

  static std::string GetDescription();

private:
  RCDAQFileReader m_reader;
  std::shared_ptr<spdlog::logger> m_log;
};

template <> double JEventSourceGeneratorT<JEventSourceRCDAQ>::CheckOpenable(std::string);
