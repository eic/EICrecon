#pragma once

namespace eicrecon {

struct IterativeVertexFinderConfig {
  int maxVertices                  = 10;
  bool reassignTracksAfterFirstFit = true;
  unsigned int minTrackHits        = 3;
};

} // namespace eicrecon
