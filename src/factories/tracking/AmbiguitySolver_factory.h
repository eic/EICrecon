// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Minjung Kim, Barak Schmookler
#pragma once

#include "algorithms/tracking/AmbiguitySolver.h"
#include "algorithms/tracking/AmbiguitySolverConfig.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"
#include <cstddef>
#include <cstdint>

namespace eicrecon {

class AmbiguitySolver_factory
    : public JOmniAlgorithmFactory<AmbiguitySolver_factory, AmbiguitySolver> {
  ParameterRef<std::uint32_t> m_maximumSharedHits{this, "maximumSharedHits",
                                                  config().maximum_shared_hits,
                                                  "Maximum number of shared hits allowed"};
  ParameterRef<std::uint32_t> m_maximumIterations{
      this, "maximumIterations", config().maximum_iterations, "Maximum number of iterations"};
  ParameterRef<std::size_t> m_nMeasurementsMin{
      this, "nMeasurementsMin", config().n_measurements_min,
      "Number of measurements required for further reconstruction"};
};

} // namespace eicrecon
