// AUTOMATICALLY GENERATED FILE - DO NOT EDIT

#ifndef EDM4NWB_ClusterDATA_H
#define EDM4NWB_ClusterDATA_H

#include "edm4eic/Cov2f.h"
#include "edm4eic/Cov3f.h"
#include "edm4hep/Vector3f.h"
#include <cstdint>

namespace edm4nwb {


/** @class ClusterData
 *  EIC hit cluster, reworked to more closely resemble EDM4hep
 *  @author: W. Armstrong, S. Joosten, C.Peng
 */
class ClusterData {
public:
  std::int32_t type{}; ///< Flagword that defines the type of the cluster
  float energy{}; ///< Reconstructed energy of the cluster [GeV].
  float energyError{}; ///< Error on the cluster energy [GeV]
  float time{}; ///< [ns]
  float timeError{}; ///< Error on the cluster time
  std::uint32_t nhits{}; ///< Number of hits in the cluster.
  ::edm4hep::Vector3f position{}; ///< Global position of the cluster [mm].
  ::edm4eic::Cov3f positionError{}; ///< Covariance matrix of the position (6 Parameters).
  float intrinsicTheta{}; ///< Intrinsic cluster propagation direction polar angle [rad]
  float intrinsicPhi{}; ///< Intrinsic cluster propagation direction azimuthal angle [rad]
  ::edm4eic::Cov2f intrinsicDirectionError{}; ///< Error on the intrinsic cluster propagation direction

  unsigned int shapeParameters_begin{};
  unsigned int shapeParameters_end{};
  unsigned int hitContributions_begin{};
  unsigned int hitContributions_end{};
  unsigned int subdetectorEnergies_begin{};
  unsigned int subdetectorEnergies_end{};
  unsigned int clusters_begin{};
  unsigned int clusters_end{};
  unsigned int hits_begin{};
  unsigned int hits_end{};
  unsigned int particleIDs_begin{};
  unsigned int particleIDs_end{};
};

} // namespace edm4eic


#endif
