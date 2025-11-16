#pragma once

#include <Acts/Definitions/Units.hpp>

namespace eicrecon {

struct SecondaryVertexFinderConfig {
  unsigned int maxVertices      = 20;
  unsigned int maxIterations    = 500;
  unsigned int maxSecIterations = 1000;
  //Max z interval used for adding tracks to fit: when adding a new vertex to the multi vertex fit,
  //only the tracks whose z at PCA is closer to the seeded vertex than tracksMaxZinterval
  //are added to this new vertex
  float tracksMaxZinterval    = 3 * Acts::UnitConstants::mm;
  float tracksMaxZintervalSec = 10 * Acts::UnitConstants::mm;

  // Max chi2 value for which tracks are considered compatible with
  // the fitted vertex. These tracks are removed from the seedTracks
  // after the fit has been performed.
  double maxVertexChi2 = 18.42;
  bool doFullSplitting = false;
  // 5 corresponds to a p-value of ~0.92 using `chi2(x=5,ndf=2)`
  float tracksMaxSignificance      = 6.7;
  float maxMergeVertexSignificance = 3;
  float minWeight                  = 1e-04;
  float maxDistToLinPoint          = 5.5 * Acts::UnitConstants::mm;
  // Bin extent in z-direction
  float spatialBinExtent         = 25 * Acts::UnitConstants::um;
  Acts::Vector4 initialVariances = Acts::Vector4{1e+2, 1e+2, 1e+2, 1e+8};
  // Bin extent in t-direction
  float temporalBinExtent          = 19. * Acts::UnitConstants::mm;
  bool doSmoothing                 = true;
  bool reassignTracksAfterFirstFit = true;
  bool useTime                     = false;
  // Use seed vertex as a constraint for the fit
  bool useSeedConstraint = true;
};

} // namespace eicrecon
