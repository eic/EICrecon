// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Sebouh Pual
#include "CalorimeterTotalFourMomentum.h"
#include <edm4eic/ClusterCollection.h>
#include <edm4hep/Vector4f.h>
#include <math.h>
#include <gsl/pointers>

/**
 Creates a pseudo particle containing the momenta and energies from all clusters in a given ClusterCollection.
 The 3-momentum for each cluster is the energy of that cluster, in the direction from the origin to the
 cluster position.  These 3-momenta, along with the energies of the clusters, are added together to form
 the total four-momentum in the calorimeter system.  

 */

namespace eicrecon {

    void CalorimeterTotalFourMomentum::init() {  }

    void CalorimeterTotalFourMomentum::process(const CalorimeterTotalFourMomentum::Input& input,
                      const CalorimeterTotalFourMomentum::Output& output) const {

      const auto [clusters] = input;
      auto [hadronic_final_state] = output;

      double Etot=0;
      double pxtot=0;
      double pytot=0;
      double pztot=0;
      for (const auto& cluster : *clusters) {
          double E = cluster.getEnergy();
          Etot+=E;
          double x= cluster.getPosition().x;
	  double y= cluster.getPosition().y;
	  double z= cluster.getPosition().z;
	  double r=sqrt(x*x+y*y+z*z);
	  pxtot+=E*x/r;
	  pytot+=E*y/r;
	  pztot+=E*z/r;
      }
      auto hfs = hadronic_final_state->create();
      hfs.setMomentum({(float)pxtot, (float)pytot, (float)pztot});
      hfs.setEnergy((float)Etot);
    }
}
