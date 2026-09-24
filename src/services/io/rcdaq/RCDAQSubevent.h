// Copyright 2024, EIC
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include <cstdint>
#include <vector>

/// Raw sub-event / packet data from an rcdaq file.
///
/// Each RCDAQSubevent corresponds to one sub-event record in the rcdaq binary
/// stream and holds the raw int32 payload words (excluding the sub-event header).
/// Downstream JANA factories decode these into EDM4hep collections.
///
/// Two IDs are carried:
///   - packet_id: the user-visible routing key.
///       PRDF format: hdrinfo & 0xFFFF (e.g. 12001 for detector packets).
///       ONCS format: equal to sub_id (sign-extended to int32).
///   - sub_id: the hardware module identifier stored in the raw header.
///       PRDF: 32-bit field.  ONCS: 16-bit field (sign-extended here).
struct RCDAQSubevent {
  int32_t packet_id{0};      ///< User-visible packet ID (decoder map key)
  int32_t sub_id{0};         ///< Hardware module ID from raw header
  int16_t sub_type{0};       ///< Sub-event type code
  int16_t sub_decoding{0};   ///< Decoding method identifier (see SubevtConstants.h)
  std::vector<int32_t> data; ///< Raw int32 payload words (payload only, no header)
};
