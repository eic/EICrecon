// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Sebouh Pual
#include "FarForwardNeutronReconstruction.h"

#include <edm4eic/ClusterCollection.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/Vector3f.h>
#include <math.h>
#include <gsl/pointers>

/**
 Creates a "neutron candidate" Reconstructed Particle consisting of all clusters in a given ClusterCollection.
 Its energy is the sum of the energies of the constituent clusters, and its direction is the direction from the
origin to the position of the most energetic cluster
 */

namespace eicrecon {

    void FarForwardNeutronReconstruction::init() {  }

    void FarForwardNeutronReconstruction::process(const FarForwardNeutronReconstruction::Input& input,
                      const FarForwardNeutronReconstruction::Output& output) const {

      const auto [clusters] = input;
      auto [out_neutrons] = output;

      double Etot=0;
      double Emax=0;
      double x=0;
      double y=0;
      double z=0;
      for (const auto& cluster : *clusters) {
          double E = cluster.getEnergy();
          Etot+=E;
          if(E>Emax){
            Emax=E;
            x=cluster.getPosition().x;
            y=cluster.getPosition().y;
            z=cluster.getPosition().z;
          }
      }
      if (Etot>0){
          auto rec_part = out_neutrons->create();
          rec_part.setEnergy(Etot);
          rec_part.setPDG(2112);
        double p=sqrt(Etot*Etot-m_neutron*m_neutron);
          double r=sqrt(x*x+y*y+z*z);
          double px=p*x/r;
          double py=p*y/r;
          double pz=p*z/r;
          rec_part.setMomentum({(float)px, (float)py, (float)pz});
          rec_part.setCharge(0);
          rec_part.setMass(m_neutron);
          for (const auto& cluster : *clusters){
            rec_part.addToClusters(cluster);
          }
      }
        //m_log->debug("Found {} neutron candidates", out_neutrons->size());

    }
}
