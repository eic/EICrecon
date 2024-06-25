#pragma once

namespace eicrecon {

struct SecondaryVertexFinderConfig {
  int  maxVertices                 = 10;
  bool reassignTracksAfterFirstFit = true;
};

} // namespace eicrecon
