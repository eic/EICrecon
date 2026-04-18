// Copyright 2024, EIC
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "JEventSourceRCDAQ.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include <JANA/JException.h>
#include <fmt/format.h>

#include "RCDAQSubevent.h"
#include "services/log/Log_service.h"

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
JEventSourceRCDAQ::JEventSourceRCDAQ(std::string resource_name, JApplication* app)
    : JEventSource(std::move(resource_name), app) {
  SetTypeName(NAME_OF_THIS);
  SetCallbackStyle(CallbackStyle::ExpertMode);

  m_log = GetApplication()->GetService<Log_service>()->logger("JEventSourceRCDAQ");
}

// ---------------------------------------------------------------------------
// Open
// ---------------------------------------------------------------------------
void JEventSourceRCDAQ::Open() {
  try {
    m_reader.open(GetResourceName());
    m_log->info("Opened rcdaq file \"{}\"", GetResourceName());
  } catch (const std::exception& e) {
    throw JException(
        fmt::format("JEventSourceRCDAQ: failed to open \"{}\": {}", GetResourceName(), e.what()));
  }
}

// ---------------------------------------------------------------------------
// Close
// ---------------------------------------------------------------------------
void JEventSourceRCDAQ::Close() {
  m_reader.close();
  m_log->info("Closed rcdaq file \"{}\"", GetResourceName());
}

// ---------------------------------------------------------------------------
// Emit
// ---------------------------------------------------------------------------
JEventSourceRCDAQ::Result JEventSourceRCDAQ::Emit(JEvent& event) {
  RCDAQFileReader::Event rcdaq_event;

  try {
    if (!m_reader.nextEvent(rcdaq_event)) {
      return Result::FailureFinished;
    }
  } catch (const std::exception& e) {
    m_log->error("Error reading rcdaq event: {}", e.what());
    return Result::FailureFinished;
  }

  event.SetRunNumber(rcdaq_event.run_number);
  event.SetEventNumber(rcdaq_event.evt_sequence);

  // Insert one RCDAQSubevent per sub-event, tagged by sub-event ID.
  // Downstream factories select sub-events by ID (e.g. event.GetSingle<RCDAQSubevent>("1234")).
  for (const auto& se : rcdaq_event.subevents) {
    auto* item     = new RCDAQSubevent(se);
    const auto tag = std::to_string(static_cast<int>(se.sub_id));
    event.Insert(item, tag);
  }

  return Result::Success;
}

// ---------------------------------------------------------------------------
// GetDescription
// ---------------------------------------------------------------------------
std::string JEventSourceRCDAQ::GetDescription() { return "rcdaq binary data file (ONCS format)"; }

// ---------------------------------------------------------------------------
// CheckOpenable
//
// Return a positive score for filenames that look like rcdaq data files.
// Common extensions used by rcdaq: .prdf, .evt, .rcdaq
// ---------------------------------------------------------------------------
template <>
double JEventSourceGeneratorT<JEventSourceRCDAQ>::CheckOpenable(std::string resource_name) {
  for (const auto* ext : {".prdf", ".evt", ".rcdaq"}) {
    if (resource_name.size() >= std::strlen(ext) &&
        resource_name.compare(resource_name.size() - std::strlen(ext), std::strlen(ext), ext) ==
            0) {
      return 0.9;
    }
  }
  return 0.0;
}
