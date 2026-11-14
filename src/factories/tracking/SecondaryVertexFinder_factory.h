// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Dongwi H. Dongwi (Bishoy)

#pragma once

#include <ActsExamples/EventData/Track.hpp>
#include <JANA/JEvent.h>
#include <cassert>
#include <edm4eic/Vertex.h>
#include <edm4eic/TrackParameters.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/tracking/SecondaryVertexFinderConfig.h"
#include "algorithms/tracking/SecondaryVertexFinder.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class SecondaryVertexFinder_factory
    : public JOmniAlgorithmFactory<SecondaryVertexFinder_factory, eicrecon::SecondaryVertexFinder,
                                   SecondaryVertexFinderConfig> {
private:
  ParameterRef<float> m_tracksMaxZinterval{this, "tracksMaxZinterval", config().tracksMaxZinterval,
                                           "Max z interval for Acts::AdaptiveMultiVertexFinder."};
  ParameterRef<unsigned int> m_maxIterations{this, "maxIterations", config().maxIterations,
                                             "Max iterations for Acts::AdaptiveMultivertexFinder"};
  ParameterRef<float> m_maxDistToLinPoint{
      this, "maxDistToLinPoint", config().maxDistToLinPoint,
      "Max disttance to line point (pca) for Acts::AdaptiveMultivertexFinder"};
  ParameterRef<double> m_maxVertexChi2{
      this, "maxVertexChi2", config().maxVertexChi2,
      "Max chi2 for a track to be considered compatible with the fitted vertex"};
  ParameterRef<float> m_tracksMaxSignificance{
      this, "tracksMaxSignificance", config().tracksMaxSignificance,
      "Max significance of a track's impact parameters w.r.t. the vertex seed"};

  Service<ACTSGeo_service> m_ACTSGeoSvc{this};
};

} // namespace eicrecon
