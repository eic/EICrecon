// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Simon Gardner

#pragma once

#include <DD4hep/DD4hepUnits.h>

namespace eicrecon {
struct FarDetectorMLReconstructionConfig {

  std::string modelPath;
  std::string methodName;

  bool requireBeamElectron{true};
};
} // namespace eicrecon
