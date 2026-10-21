// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <string>
#include <vector>
#include <Evaluator/DD4hepUnits.h>
#include <edm4eic/unit_system.h>

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

  // List of PDGs that are treated as promptly decaying
  std::vector<int> promptDecayPDGs{111, 221, 331, 310, 3122};

  // if > 0, the MC truth of a hit only uses the deposits within this time window (ns)
  // after the hit time; use the same window as in CalorimeterHitDigi
  double crossingWindow{0.0 * edm4eic::unit::ns};
  double crossingMargin{0.01 * edm4eic::unit::ns};
};

} // namespace eicrecon
