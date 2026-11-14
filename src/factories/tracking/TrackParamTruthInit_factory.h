// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <JANA/JEvent.h>
#include <edm4eic/TrackParametersCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "services/geometry/acts/ACTSGeo_service.h"
#include "algorithms/tracking/TrackParamTruthInit.h"
#include "algorithms/tracking/TrackParamTruthInitConfig.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class TrackParamTruthInit_factory
    : public JOmniAlgorithmFactory<TrackParamTruthInit_factory, eicrecon::TrackParamTruthInit,
                                   TrackParamTruthInitConfig> {
private:
  ParameterRef<double> m_maxVertexX{this, "MaxVertexX", config().maxVertexX,
                                    "Maximum abs(vertex x) for truth tracks turned into seed"};
  ParameterRef<double> m_maxVertexY{this, "MaxVertexY", config().maxVertexY,
                                    "Maximum abs(vertex y) for truth tracks turned into seed"};
  ParameterRef<double> m_maxVertexZ{this, "MaxVertexZ", config().maxVertexZ,
                                    "Maximum abs(vertex z) for truth tracks turned into seed"};
  ParameterRef<double> m_minMomentum{this, "MinMomentum", config().minMomentum,
                                     "Minimum momentum for truth tracks turned into seed"};
  ParameterRef<double> m_maxEtaForward{
      this, "MaxEtaForward", config().maxEtaForward,
      "Maximum forward abs(eta) for truth tracks turned into seed"};
  ParameterRef<double> m_maxEtaBackward{
      this, "MaxEtaBackward", config().maxEtaBackward,
      "Maximum backward abs(eta) for truth tracks turned into seed"};
  ParameterRef<double> m_momentumSmear{
      this, "MomentumSmear", config().momentumSmear,
      "Momentum magnitude fraction to use as width of gaussian smearing"};
  ParameterRef<bool> m_useVertexAsPerigee{
      this, "UseVertexAsPerigee", config().useVertexAsPerigee,
      "Anchor the seed perigee at the MCParticle vertex (for displaced tracks)"};

  Service<ACTSGeo_service> m_ACTSGeoSvc{this};
};

} // namespace eicrecon
