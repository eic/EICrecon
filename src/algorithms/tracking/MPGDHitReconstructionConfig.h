// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Sylvester Joosten, Wouter Deconinck, Dmitry Romanov

#pragma once

namespace eicrecon {
struct MPGDHitReconstructionConfig {
  // sub-systems should overwrite their own (see "detectors/MPGD/MPGD.cc")

  // Readout identifiers for dividing detector
  std::string readout{""};
  float timeResolution = 10;
  std::array<float,2> stripResolutions = {150 * dd4hep::um, 150 * dd4hep::um};
};
} // namespace eicrecon
