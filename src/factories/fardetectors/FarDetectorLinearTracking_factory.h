// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 - 2025, Simon Gardner

#pragma once

#include "services/geometry/dd4hep/DD4hep_service.h"

// Event Model related classes
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/MCRecoTrackerHitLinkCollection.h>
#include <edm4eic/MCRecoTrackParticleAssociationCollection.h>
#include <edm4eic/TrackCollection.h>
#include <edm4eic/Measurement2DCollection.h>
#include <algorithms/fardetectors/FarDetectorLinearTracking.h>

#include "extensions/jana/JOmniAlgorithmFactory.h"
#include <spdlog/logger.h>

namespace eicrecon {

class FarDetectorLinearTracking_factory
    : public JOmniAlgorithmFactory<FarDetectorLinearTracking_factory,
                                   eicrecon::FarDetectorLinearTracking,
                                   FarDetectorLinearTrackingConfig> {
private:
  ParameterRef<std::size_t> n_layer{this, "numLayers", config().n_layer};
  ParameterRef<std::vector<double>> layer_weights{this, "layerWeights", config().layer_weights};
  ParameterRef<std::size_t> layer_hits_max{this, "layerHitsMax", config().layer_hits_max};
  ParameterRef<float> chi2_max{this, "chi2Max", config().chi2_max};
};

} // namespace eicrecon
