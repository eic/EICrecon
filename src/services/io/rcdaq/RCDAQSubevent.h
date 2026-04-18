// Copyright 2024, EIC
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <cstdint>
#include <vector>

/// Raw sub-event data from an rcdaq file.
///
/// Each RCDAQSubevent corresponds to one sub-event record in the rcdaq binary
/// stream and holds the raw int32 payload words (excluding the sub-event header).
/// Downstream JANA factories decode these into EDM4hep collections.
struct RCDAQSubevent {
  int16_t sub_id{0};         ///< Sub-event identifier (hardware module ID)
  int16_t sub_type{0};       ///< Sub-event type code
  int16_t sub_decoding{0};   ///< Decoding method identifier (see SubevtConstants.h)
  std::vector<int32_t> data; ///< Raw int32 payload words (payload only, no header)
};
