// Copyright 2024, EIC
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <podio/CollectionBuffers.h>
#include <podio/CollectionIDTable.h>
#include <podio/GenericParameters.h>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "RCDAQDecoder.h"
#include "RCDAQFileReader.h"

/// Adapter between a raw rcdaq event and the podio::FrameDataType concept.
///
/// RCDAQFrameData wraps an RCDAQFileReader::Event together with a map of
/// registered RCDAQDecoder instances and presents the combined data through the
/// four methods required by the podio::FrameDataType concept.  The podio::Frame
/// calls getCollectionBuffers() lazily — only when a specific collection is
/// actually requested — so decoders are invoked on-demand.
///
/// Typical construction (inside JEventSourceRCDAQ::Emit):
/// @code
///   auto frame_data = std::make_unique<RCDAQFrameData>(
///       std::move(rcdaq_event), m_decoder_map);
///   auto frame = std::make_unique<podio::Frame>(std::move(frame_data));
///   event.Insert(frame.release());
/// @endcode
class RCDAQFrameData {
public:
  /// Map from sub-event ID to (non-owning) decoder pointer.
  using DecoderMap = std::unordered_map<int16_t, RCDAQDecoder*>;

  /// Construct from a parsed rcdaq event and an external decoder map.
  ///
  /// \p decoders is not owned; the caller (JEventSourceRCDAQ) must keep the
  /// decoder objects alive for the lifetime of this object.
  RCDAQFrameData(RCDAQFileReader::Event event, const DecoderMap& decoders);

  // podio::FrameDataType concept requirements ---------------------------------

  /// Returns a snapshot of the collection-name → ID table built from all
  /// sub-events that have a registered decoder.
  podio::CollectionIDTable getIDTable() const;

  /// Decode and return the CollectionReadBuffers for collection \p name.
  /// Returns nullopt if no matching sub-event / decoder is found.
  std::optional<podio::CollectionReadBuffers> getCollectionBuffers(const std::string& name);

  /// Returns the collection names that can be decoded from this event.
  std::vector<std::string> getAvailableCollections() const;

  /// Returns event-level metadata: "run_number" and "event_sequence" as int.
  std::unique_ptr<podio::GenericParameters> getParameters();

private:
  RCDAQFileReader::Event m_event;
  const DecoderMap& m_decoders;

  /// ID table built at construction time over decodable collections.
  podio::CollectionIDTable m_idTable;

  /// Maps collection name → pointer into m_event.subevents (stable after construction).
  std::unordered_map<std::string, const RCDAQSubevent*> m_nameToSubevent;

  /// Event-level parameters (consumed once by the Frame constructor).
  std::unique_ptr<podio::GenericParameters> m_params;
};
