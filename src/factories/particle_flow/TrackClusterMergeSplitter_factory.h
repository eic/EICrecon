// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Derek Anderson

#pragma once

#include <edm4eic/EDM4eicVersion.h>
#include <string>

#include "extensions/jana/JOmniAlgorithmFactory.h"
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "algorithms/particle_flow/TrackClusterMergeSplitter.h"

namespace eicrecon {

class TrackClusterMergeSplitter_factory
    : public JOmniAlgorithmFactory<TrackClusterMergeSplitter_factory,
                                   eicrecon::TrackClusterMergeSplitter,
                                   TrackClusterMergeSplitterConfig> {
private:
  // parameter bindings
  ParameterRef<double> m_minSigCut{this, "minSigCut", config().minSigCut};
  ParameterRef<double> m_avgEP{this, "avgEP", config().avgEP};
  ParameterRef<double> m_sigEP{this, "sigEP", config().sigEP};
  ParameterRef<double> m_drAdd{this, "drAdd", config().drAdd};
  ParameterRef<uint64_t> m_surfaceToUse{this, "surfaceToUse", config().surfaceToUse};
  ParameterRef<double> m_transverseEnergyProfileScale{this, "transverseEnergyProfileScale",
                                                      config().transverseEnergyProfileScale};
}; // end TrackClusterMergeSplitter_factory

} // namespace eicrecon
