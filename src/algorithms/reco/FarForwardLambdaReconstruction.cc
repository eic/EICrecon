// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Sebouh Paul

#include "TLorentzVector.h"
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <math.h>
#include <gsl/pointers>
#include <stdexcept>
#include <vector>

#include "FarForwardLambdaReconstruction.h"

/**
Creates Lambda candidates from a neutron and two photons from a pi0 decay
 */

namespace eicrecon {

    void FarForwardLambdaReconstruction::init() {
      
    }

  /* converts one type of vector format to another */
  void toTVector3(TVector3& v1, const edm4hep::Vector3f& v2){
    v1.SetXYZ(v2.x,v2.y,v2.z);
  }

  
    void FarForwardLambdaReconstruction::process(const FarForwardLambdaReconstruction::Input& input,
                      const FarForwardLambdaReconstruction::Output& output) const {
      const auto [neutrons, gammas] = input;
      auto [out_lambdas, out_decay_products] = output;

      if (neutrons->size()!=1 || gammas->size()!=2)
	return;
      
      double En=(*neutrons)[0].getEnergy();
      double pn=sqrt(En*En-m_neutron*m_neutron);
      double E1=(*gammas)[0].getEnergy();
      double E2=(*gammas)[1].getEnergy();
      TVector3 xn;
      TVector3 x1;
      TVector3 x2;
      
      toTVector3(xn,(*neutrons)[0].getReferencePoint());
      toTVector3(x1,(*gammas)[0].getReferencePoint());
      toTVector3(x2,(*gammas)[1].getReferencePoint());

      xn.RotateY(-m_cfg.rotY);
      x1.RotateY(-m_cfg.rotY);
      x2.RotateY(-m_cfg.rotY);

      TVector3 vtx(0,0,0);
      double f=0;
      double df=0.5;
      double theta_open_expected=2*asin(m_pi0/(2*sqrt(E1*E2)));
      for(int i = 0; i<m_cfg.iterations; i++){
	TLorentzVector n(pn*(xn-vtx).Unit(), En);
	TLorentzVector g1(E1*(x1-vtx).Unit(), E1);
	TLorentzVector g2(E2*(x1-vtx).Unit(), E2);
	double theta_open=g1.Angle(g2.Vect());
	TLorentzVector lambda=n+g1+g2;
	if (theta_open>theta_open_expected)
	  f-=df;
	else if (theta_open<theta_open_expected)
	  f+=df;

	vtx=lambda.Vect()*(f*m_cfg.zmax/lambda.Z());
	df/=2;

	if (i==m_cfg.iterations-1){
	  double mass_rec=lambda.M();
	  if (abs(mass_rec-m_lambda)>m_cfg.lambda_max_mass_dev)
	    return;
	  
	  // rotate everything back to the lab coordinates.
	  vtx.RotateY(m_cfg.rotY);
	  lambda.RotateY(m_cfg.rotY);
	  n.RotateY(m_cfg.rotY);
	  g1.RotateY(m_cfg.rotY);
	  g2.RotateY(m_cfg.rotY);
	  
	  auto b=-lambda.BoostVector();
	  n.Boost(b);
	  g1.Boost(b);
	  g2.Boost(b);
	  
	  
	  auto rec_part = out_lambdas->create();
	  rec_part.setPDG(3122);
	  //edm4hep::Vector3f position(vtx.X(), vtx.Y(), vtx.Z());
	  //edm4hep::Vector3f momentum(lambda.X(), lambda.Y(), lambda.Z());
	  
	  rec_part.setEnergy(lambda.E());
	  rec_part.setMomentum({static_cast<float>(lambda.X()), static_cast<float>(lambda.Y()), static_cast<float>(lambda.Z())});
	  rec_part.setReferencePoint({static_cast<float>(vtx.X()), static_cast<float>(vtx.Y()), static_cast<float>(vtx.Z())});
	  rec_part.setCharge(0);
	  rec_part.setMass(mass_rec);

	  rec_part = out_decay_products->create();
          rec_part.setPDG(2112);
          rec_part.setEnergy(n.E());
          rec_part.setMomentum({static_cast<float>(n.X()), static_cast<float>(n.Y()), static_cast<float>(n.Z())});
          rec_part.setReferencePoint({static_cast<float>(vtx.X()), static_cast<float>(vtx.Y()), static_cast<float>(vtx.Z())});
          rec_part.setCharge(0);
          rec_part.setMass(m_neutron);

	  rec_part = out_decay_products->create();
          rec_part.setPDG(22);
          rec_part.setEnergy(g1.E());
          rec_part.setMomentum({static_cast<float>(g1.X()), static_cast<float>(g1.Y()), static_cast<float>(g1.Z())});
          rec_part.setReferencePoint({static_cast<float>(vtx.X()), static_cast<float>(vtx.Y()), static_cast<float>(vtx.Z())});
          rec_part.setCharge(0);
          rec_part.setMass(0);

	  rec_part = out_decay_products->create();
          rec_part.setPDG(22);
          rec_part.setEnergy(g2.E());
          rec_part.setMomentum({static_cast<float>(g2.X()), static_cast<float>(g2.Y()), static_cast<float>(g2.Z())});
          rec_part.setReferencePoint({static_cast<float>(vtx.X()), static_cast<float>(vtx.Y()), static_cast<float>(vtx.Z())});
          rec_part.setCharge(0);
          rec_part.setMass(0);
	  
	  return;
	  
	}	
      }
    }
}
