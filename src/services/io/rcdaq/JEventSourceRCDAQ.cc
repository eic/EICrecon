// Copyright 2024, EIC
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "JEventSourceRCDAQ.h"

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include <JANA/JException.h>
#include <podio/Frame.h>
#include <fmt/format.h>
#include <algorithm>

#include "RCDAQFrameData.h"
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
// addDecoder
// ---------------------------------------------------------------------------
void JEventSourceRCDAQ::addDecoder(std::unique_ptr<RCDAQDecoder> decoder) {
  const int32_t id = decoder->packetID();
  m_decoders[id]   = std::move(decoder);
}

// ---------------------------------------------------------------------------
// Open
// ---------------------------------------------------------------------------
void JEventSourceRCDAQ::Open() {
  GetApplication()->SetDefaultParameter("rcdaq:dump", m_dump,
                                        "Print raw sub-event packet headers and payload words "
                                        "without decoding (useful for exploring new files)");

  // Build the non-owning decoder map that will be shared across all frames.
  m_decoder_map.clear();
  for (auto& [id, dec] : m_decoders) {
    m_decoder_map[id] = dec.get();
  }

  if (!m_decoders.empty()) {
    m_log->info("Registered {} rcdaq decoder(s):", m_decoders.size());
    for (const auto& [id, dec] : m_decoders) {
      m_log->info("  packet_id {:5d} → {} ({})", id, dec->collectionName(), dec->collectionType());
    }
  } else {
    m_log->warn("No rcdaq decoders registered; Frame will contain no collections.");
  }

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

  if (m_dump) {
    m_log->info("[rcdaq] evt_seq={} run={} format={} npackets={}", rcdaq_event.evt_sequence,
                rcdaq_event.run_number,
                (m_reader.format() == RCDAQFileReader::Format::PRDF ? "PRDF" : "ONCS"),
                rcdaq_event.subevents.size());
    for (const auto& se : rcdaq_event.subevents) {
      // Build hex dump of first 8 payload words
      std::string hex;
      const int nprint = static_cast<int>(std::min(se.data.size(), std::size_t{8}));
      for (int i = 0; i < nprint; i++) {
        hex += fmt::format(" {:08x}", static_cast<uint32_t>(se.data[i]));
      }
      if (static_cast<int>(se.data.size()) > nprint) {
        hex += " ...";
      }
      m_log->info("  packet_id={:5d} (0x{:04x})  sub_id={:5d}  len={:6d}w"
                  "  decoding={:3d}  type={:2d}  payload[0..{}]:{}",
                  se.packet_id, static_cast<uint32_t>(se.packet_id), se.sub_id,
                  static_cast<int>(se.data.size()), static_cast<int>(se.sub_decoding),
                  static_cast<int>(se.sub_type), nprint - 1, hex);
    }
  }

  // Wrap the raw event in a FrameData object that satisfies podio::FrameDataType.
  // The Frame will call RCDAQFrameData::getCollectionBuffers() lazily when a
  // collection is first accessed, invoking the appropriate RCDAQDecoder.
  auto frame_data = std::make_unique<RCDAQFrameData>(std::move(rcdaq_event), m_decoder_map);
  auto frame      = std::make_unique<podio::Frame>(std::move(frame_data));
  event.Insert(frame.release());

  return Result::Success;
}

// ---------------------------------------------------------------------------
// GetDescription
// ---------------------------------------------------------------------------
std::string JEventSourceRCDAQ::GetDescription() {
  return "rcdaq binary data file (ONCS/PRDF format)";
}

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
