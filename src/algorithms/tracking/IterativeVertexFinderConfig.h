#pragma once

namespace eicrecon {

struct IterativeVertexFinderConfig {
  int m_maxVertices                  = 10;
  bool m_reassignTracksAfterFirstFit = true;
};

} // namespace eicrecon
