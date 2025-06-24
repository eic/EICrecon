#pragma once

#include <Acts/Definitions/Units.hpp>

namespace eicrecon {

struct SecondaryVertexFinderConfig {
  int maxVertices                  = 20;
  float maxIterations              = 500;
  //Max z interval used for adding tracks to fit: when adding a new vertex to the multi vertex fit,
  //only the tracks whose z at PCA is closer to the seeded vertex than tracksMaxZinterval
  //are added to this new vertex
  float tracksMaxZinterval         = 35 * Acts::UnitConstants::mm;
  bool doFullSplitting             = false;
  // 5 corresponds to a p-value of ~0.92 using `chi2(x=5,ndf=2)`
  float tracksMaxSignificance      = 6.7;
  float maxMergeVertexSignificance = 5;
  float minWeight                  = 1e-04;
  float maxDistToLinPoint          = 5.5 * Acts::UnitConstants::mm;
  // Bin extent in z-direction
  float spatialBinExtent           = 25 * Acts::UnitConstants::um;
  // Bin extent in t-direction
  float temporalBinExtent          = 19. * Acts::UnitConstants::mm;
  bool doSmoothing                 = true;
  bool reassignTracksAfterFirstFit = true;
  bool useTime                     = false;
};

} // namespace eicrecon
