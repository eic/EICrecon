// Created by Joe Osborn
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <ActsExamples/EventData/Track.hpp>
#include <JANA/JEvent.h>
#include <cassert>
#include <edm4eic/VertexCollection.h>
#include <edm4eic/ReconstructedParticle.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/tracking/IterativeVertexFinderConfig.h"
#include "algorithms/tracking/IterativeVertexFinder.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

class IterativeVertexFinder_factory
    : public JOmniAlgorithmFactory<IterativeVertexFinder_factory, eicrecon::IterativeVertexFinder,
                                   IterativeVertexFinderConfig> {
private:
  ParameterRef<int> m_maxVertices{this, "maxVertices", config().maxVertices,
                                  "Maximum num vertices that can be found"};
  ParameterRef<bool> m_reassignTracksAfterFirstFit{
      this, "reassignTracksAfterFirstFit", config().reassignTracksAfterFirstFit,
      "Whether or not to reassign tracks after first fit"};
  ParameterRef<unsigned int> m_minTrackHits{
      this, "minTrackHits", config().minTrackHits,
      "Minimum number of hits to require for the tracks used"};

  Service<ACTSGeo_service> m_ACTSGeoSvc{this};
};

} // namespace eicrecon
