#pragma once

namespace eicrecon {

struct IterativeVertexFinderConfig {
  int maxVertices                  = 10;
  bool reassignTracksAfterFirstFit = true;
};

} // namespace eicrecon
