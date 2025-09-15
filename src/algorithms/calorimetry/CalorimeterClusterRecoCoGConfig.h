// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <string>
#include <vector>
#include <Evaluator/DD4hepUnits.h>

#include "ClusterTypes.h"

namespace eicrecon {

struct CalorimeterClusterRecoCoGConfig {

  std::string energyWeight;

  double sampFrac      = 1.;
  double logWeightBase = 3.6;

  //optional:  have the log weight base depend on the energy
  //  logWeightBaseCoeffs[0]+logWeightBaseCoeffs[1]*l+logWeightBaseCoeffs[2]*l*l + ...
  // where l = log(cl.getEnergy()/logWeightBase_Eref)
  // If this is empty, use the logWeightBase parameter for backwards compatibility.
  std::vector<double> logWeightBaseCoeffs{};
  double logWeightBase_Eref = 50 * dd4hep::MeV;

  // Constrain the cluster position eta to be within
  // the eta of the contributing hits. This is useful to avoid edge effects
  // for endcaps.
  bool enableEtaBounds = false;

  // cluster type: can be used to flag clusters
  // as being a type specified by the Jug::Reco::ClusterType
  // enum in ClusterTypes.h. This can be useful for, eg.,
  // flagging EMCal vs. HCal clusters in downstream algorithms
  int32_t clusterType = Jug::Reco::ClusterType::kCluster2D;

};

} // namespace eicrecon
