#pragma once

#include "Acts/Vertexing/VertexingOptions.hpp"
#include "Acts/Vertexing/IterativeVertexFinder.hpp"


namespace eicrecon {

  struct IVFConfig {
    int m_maxVertices = 10;
    bool m_reassignTracksAfterFirstFit = true;
  };

}
