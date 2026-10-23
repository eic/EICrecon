// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2026 Whitney Armstrong, Sylvester Joosten, Wouter Deconinck, Dmitry Romanov, Yann Bedfer

#pragma once

#include <edm4eic/unit_system.h>

namespace eicrecon {
struct MPGDHitReconstructionConfig {
  // sub-systems should overwrite their own (see "detectors/MPGD/MPGD.cc")

  // Readout identifiers for dividing detector
  std::string readout{""};
  // Assigned as TrackerHit::timeError; should match the digitization time smearing
  double timeResolution                 = 10 * edm4eic::unit::ns;
  std::array<float, 2> stripResolutions = {150 * dd4hep::um, 150 * dd4hep::um};
};
} // namespace eicrecon
