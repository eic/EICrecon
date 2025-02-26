// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Derek Anderson

#pragma once

#include <string>
#include <vector>
#include <Evaluator/DD4hepUnits.h>

namespace eicrecon {

  struct CalorimeterClusterShapeConfig {

    //! determines if intrinsic theta/phi are calculated
    bool longitudinalShowerInfoAvailable = false;

    //! weighting method to use
    std::string energyWeight = "none";

    //! optional parameters for having the log weight base
    //! depend on the energy via
    //!
    //!     logweightBaseCoeffs[0]+logweightBaseCoeffs[1]*l...
    //!
    //! where l = log(clusterEnergy/logWeightBase_Eref).
    //! If this is empty, the logWeightBase parameter will
    //! be used for backwards compatibility.
    std::vector<double> logWeightBaseCoeffs{};
    double logWeightBase_Eref = 50 * dd4hep::MeV;
    double logWeightBase = 3.6;

  };  // end CalorimeterClusterShapeConfig

}  // namespace eicrecon
