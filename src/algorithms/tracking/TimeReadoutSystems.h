// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#pragma once

#include <cstdint>
#include <set>
#include <string>
#include <vector>

namespace dd4hep {
class Detector;
}

namespace eicrecon {

/// Resolve DD4hep readout names to the 8-bit system IDs of the detectors that use them.
/// The system ID matches the extra field of the Acts GeometryIdentifier of their surfaces.
/// Throws std::runtime_error if a readout name is not used by any detector.
std::set<std::uint8_t> timeSystemIDsForReadouts(const dd4hep::Detector& detector,
                                                const std::vector<std::string>& readouts);

} // namespace eicrecon
