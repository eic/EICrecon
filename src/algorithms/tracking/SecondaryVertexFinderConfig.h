#pragma once

namespace eicrecon {

struct SecondaryVertexFinderConfig {
  int  maxVertices                 = 10;
  bool reassignTracksAfterFirstFit = true;
  bool useTime=true;
};

} // namespace eicrecon
