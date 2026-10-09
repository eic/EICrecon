// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#include "TimeReadoutSystems.h"

#include <DD4hep/DetElement.h>
#include <DD4hep/Detector.h>
#include <DD4hep/Readout.h>
#include <fmt/format.h>
#include <algorithm>
#include <functional>
#include <stdexcept>
#include <utility>

namespace eicrecon {

std::set<std::uint8_t> timeSystemIDsForReadouts(const dd4hep::Detector& detector,
                                                const std::vector<std::string>& readouts) {
  std::set<std::uint8_t> systemIDs;
  for (const auto& readout : readouts) {
    // Sensitive detectors are named after the DetElement they belong to
    std::vector<std::string> detectorNames;
    for (const auto& [name, handle] : detector.sensitiveDetectors()) {
      const dd4hep::SensitiveDetector sd{handle};
      if (sd.isValid() && sd.readout().isValid() && sd.readout().name() == readout) {
        detectorNames.push_back(name);
      }
    }

    // DetElements may be nested in assemblies, so search the whole tree by name
    bool found                                                 = false;
    const std::function<void(const dd4hep::DetElement&)> visit = [&](const dd4hep::DetElement& de) {
      if (std::find(detectorNames.begin(), detectorNames.end(), de.name()) != detectorNames.end()) {
        systemIDs.insert(static_cast<std::uint8_t>(0xff & de.id()));
        found = true;
        return;
      }
      for (const auto& [_, child] : de.children()) {
        visit(child);
      }
    };
    visit(detector.world());

    if (!found) {
      throw std::runtime_error(
          fmt::format("Readout \"{}\" is not used by any detector in the geometry", readout));
    }
  }
  return systemIDs;
}

} // namespace eicrecon
