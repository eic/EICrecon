// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck, Sylvester Joosten, Dmitry Romanov

#pragma once

#include <DD4hep/DD4hepUnits.h>
#include <vector>
#include <utility>

namespace eicrecon {

enum class SubdetectorRegion { barrel, backward, forward };

struct SiliconTrackerDigiConfig {
  // sub-systems should overwrite their own
  // NB: be aware of thresholds in npsim! E.g. https://github.com/eic/npsim/pull/9/files
  double threshold      = 0 * dd4hep::keV;
  double timeResolution = 8; /// TODO 8 of what units??? Same TODO in juggler. Probably [ns]
  bool addNoise         = true;
  // A vector of pairs to specify target layers, where each pair consists of
  // a SubdetectorRegion and a layer index (int).
  std::vector<std::pair<SubdetectorRegion, int>> target_layers = {
    {SubdetectorRegion::barrel, 4},
    {SubdetectorRegion::forward, 3},
    {SubdetectorRegion::backward, 1}
};
};

} // namespace eicrecon
