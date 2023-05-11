#pragma once

#include "Acts/Vertexing/IterativeVertexFinder.hpp"
#include "Acts/Vertexing/VertexingOptions.hpp"

namespace eicrecon {

struct IVFConfig {
  int m_maxVertices                  = 10;
  bool m_reassignTracksAfterFirstFit = true;
};

} // namespace eicrecon
