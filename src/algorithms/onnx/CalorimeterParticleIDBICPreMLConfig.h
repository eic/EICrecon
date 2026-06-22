// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Tomas Sosa, Wouter Deconinck

#pragma once

namespace eicrecon {

struct CalorimeterParticleIDBICPreMLConfig {
  int nLayers          = 12;   // total merged layers seen by the ONNX model
  int nHits            = 50;   // hits kept per layer
  int scifiLayerOffset = 6;    // should match max Astropix layer count used in training

  float r0Min  = 500.F;
  float r0Max  = 2000.F;
  float etaMin = -0.3F;
  float etaMax = 0.3F;
  float phiMin = -0.4F;
  float phiMax = 0.4F;

  double maxMatchDeltaR = 0.15; // ScFi cluster ↔ Imaging cluster matching window
};

} // namespace eicrecon