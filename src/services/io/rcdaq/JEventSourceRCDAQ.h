// Copyright 2024, EIC
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <JANA/JApplicationFwd.h>
#include <JANA/JEventSource.h>
#include <JANA/JEventSourceGeneratorT.h>
#include <spdlog/logger.h>
#include <memory>
#include <string>
#include <unordered_map>

#include "RCDAQDecoder.h"
#include "RCDAQFileReader.h"
#include "RCDAQFrameData.h"

/// JANA2 event source for rcdaq binary data files.
///
/// Reads ONCS-format rcdaq files and, for every DATA event, constructs a
/// podio::Frame backed by an RCDAQFrameData object and inserts it into the
/// JEvent.  Collections are decoded lazily: a registered RCDAQDecoder is
/// called only when the Frame is asked for the corresponding collection.
///
/// Register decoders before the source is opened (e.g. in InitPlugin):
/// @code
///   auto src = std::make_shared<JEventSourceRCDAQ>("myfile.prdf", app);
///   src->addDecoder(std::make_unique<MyCaloDecoder>());
///   app->Add(src);
/// @endcode
///
/// Downstream code accesses collections through the Frame:
/// @code
///   auto* frame = event.GetSingle<podio::Frame>();
///   auto& hits  = frame->get<edm4hep::RawCalorimeterHitCollection>("CaloHits");
/// @endcode
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

  /// Register a decoder.  Ownership is transferred to this event source.
  /// Must be called before Open().
  void addDecoder(std::unique_ptr<RCDAQDecoder> decoder);

private:
  RCDAQFileReader m_reader;
  std::shared_ptr<spdlog::logger> m_log;

  /// Owned decoders, keyed by sub-event ID.
  std::unordered_map<int16_t, std::unique_ptr<RCDAQDecoder>> m_decoders;

  /// Non-owning view of m_decoders, rebuilt in Open() and passed by const-ref
  /// to each RCDAQFrameData instance.
  RCDAQFrameData::DecoderMap m_decoder_map;
};

template <> double JEventSourceGeneratorT<JEventSourceRCDAQ>::CheckOpenable(std::string);
