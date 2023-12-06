// Copyright (C) 2023 Sebouh Paul
// SPDX-License-Identifier: LGPL-3.0-or-later

// References:
//   https://arxiv.org/abs/2308.06939


#include <DD4hep/Readout.h>
#include <Evaluator/DD4hepUnits.h>
#include <TInterpreter.h>
#include <TInterpreterValue.h>
#include <edm4hep/Vector3d.h>
#include <Eigen/Dense>
#include <fmt/format.h>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include <iostream>

#include "HEXPLIT.h"

#include "algorithms/calorimetry/HEXPLITConfig.h"

//using namespace edm4eic;

namespace eicrecon {



void HEXPLIT::init(const dd4hep::Detector* detector, std::shared_ptr<spdlog::logger>& logger) {
    m_log = logger;
    m_detector = detector;
}


  
  //void subcell_reweight(TTreeReaderArray<float>&E,TTreeReaderArray<float>&t,TTreeReaderArray<float>&x,TTreeReaderArray<float>&y,TTreeReaderArray<float>&z, //number of hits, energies, time, and local x,y,z positions of the hits
  //		      vector<double> & subcellE, vector<double>& subcellx, vector<double> & subcelly, vector<double> & subcellz,  //returned position of the cluster
  //		  hexplit_opts opts) {
std::unique_ptr<edm4eic::CalorimeterHitCollection> HEXPLIT::process(const edm4eic::CalorimeterHitCollection &hits){  
  int nhits=hits.size();
  double sl=m_cfg.side_length;
  double layer_spacing=m_cfg.layer_spacing;
  double Emin=m_cfg.Emin;
  double tmax=m_cfg.tmax;
  double MIP=m_cfg.MIP;

  double x[nhits];
  double y[nhits];
  double z[nhits];
  double E[nhits];
  double t[nhits];

  for(int i=0; i<nhits; i++){
    x[i]=hits[i].getLocal().x;
    y[i]=hits[i].getLocal().y;
    z[i]=hits[i].getLocal().z;
    E[i]=hits[i].getEnergy();
    t[i]=hits[i].getTime();
  }

  auto subcellHits = std::make_unique<edm4eic::CalorimeterHitCollection>();
  double Esum=0;
  for(int i=0; i<nhits; i++){
    //skip hits that do not pass E and t cuts
    if (E[i]<Emin || t[i]>tmax)
      continue;
    
    //keep track of the energy in each neighboring cell
    double Eneighbors[SUBCELLS];
    for (int j=0; j<SUBCELLS; j++)
      Eneighbors[j]=0;
      
    for (int j=0; j<nhits; j++){
      //only look at hits nearby within two layers of the current layer
      if (abs(z[i]-z[j])>2.5*layer_spacing || z[i]==z[j])
	continue;
      //cout << "z cut" << endl;
      if (E[j]<Emin || t[j]>tmax)
	continue;
      //difference in transverse position (in units of side lengths)
      double dx=(x[j]-x[i])/sl;
      double dy=(y[j]-y[i])/sl;
      if (abs(dx)>2 || abs(dy)>sqrt(3))
	continue;
      //cout << "dx, dy cut passed: " << dx << " " << dy <<  endl;
      
      //loop over locations of the neighboring cells
      //and check if the jth hit matches this location
      double tol=0.01; //tolerance for rounding errors
      for(int k=0;k<SUBCELLS;k++){
	//cout << "neighbor pos"<< neighbor_offsets_x[k] << " " << neighbor_offsets_y[k] << endl;
	if(abs(dx-neighbor_offsets_x[k])<tol && abs(dy-neighbor_offsets_y[k])<tol){
	  //cout << "found neighbor " << k;
	  Eneighbors[k]+=E[j];
	  break;
	}
      }
    }
    double weights[SUBCELLS];
    for(int k=0; k<SUBCELLS; k++){
      Eneighbors[k]=std::max(Eneighbors[k],MIP);
    }
    double sum_weights=0;
    for(int k=0; k<SUBCELLS; k++){
      weights[k]=Eneighbors[neighbor_indices[k][0]]*Eneighbors[neighbor_indices[k][1]]*Eneighbors[neighbor_indices[k][2]];
      sum_weights+=weights[k];
    }
    for(int k=0; k<SUBCELLS;k++){

      //create the subcell hits

      Eigen::Vector3d local(x[i]+subcell_offsets_x[k]*sl, y[i]+subcell_offsets_y[k]*sl, z[i]);
      const decltype(edm4eic::CalorimeterHitData::local) local_position(local[0], local[1], local[2]);

      //also convert this to the detector's global coordinates
      Eigen::AngleAxisd rollAngle(m_cfg.rot_z, Eigen::Vector3d::UnitZ());
      Eigen::AngleAxisd yawAngle(m_cfg.rot_y, Eigen::Vector3d::UnitY());
      Eigen::AngleAxisd pitchAngle(m_cfg.rot_x, Eigen::Vector3d::UnitX());

      Eigen::Quaternion<double> q = rollAngle * yawAngle * pitchAngle;
      
      Eigen::Matrix3d rotationMatrix = q.matrix();

      Eigen::Vector3d translation(m_cfg.trans_x, m_cfg.trans_y, m_cfg.trans_z);

      auto gpos = rotationMatrix*local+translation;

      const decltype(edm4eic::CalorimeterHitData::position) position(gpos[0], gpos[1], gpos[2]);
      
      //bounding box dimensions depend on the orientation of the rhombus
      int orientation = k%3==0;
      const decltype(edm4eic::CalorimeterHitData::dimension) dimension(sl*(orientation?1:1.5), sl*sqrt(3)/2.*(orientation?2:1),
								       hits[i].getDimension()[2]);
      
      subcellHits->create(
	    hits[i].getCellID(),
	    E[i]*weights[k]/sum_weights,
            0,
            hits[i].getTime(),
            0,
            position,
            dimension,
            hits[i].getSector(),
            hits[i].getLayer(),
	    local_position);
      //subcellE.push_back(E[i]*weights[k]/sum_weights);
      //subcellx.push_back(x[i]+subcell_offsets_x[k]*sl);
      //subcelly.push_back(y[i]+subcell_offsets_y[k]*sl);
      //subcellz.push_back(z[i]);
    }
  }
  return subcellHits;
}


} // namespace eicrecon

/*
// Optional subroutine to merge all hits or subcells at a given transverse position together after doing
// subcell reweighting.  This method was found to not improve the angle resolution, so it is not
// used.  
void flatten(vector<double>&E, vector<double>& x, vector<double>& y, vector<double>& z,
	     vector<double>&Enew,vector<double>& xnew, vector<double>& ynew, vector<double>& znew){
  Enew.clear();
  xnew.clear();
  ynew.clear();
  znew.clear();
  for(int i = 0; i<E.size(); i++){
    int found=0;
    for(int j=0; j<Enew.size(); j++){
      double tol=0.01;
      if(abs(xnew[j]-x[i])<tol && abs(ynew[j]-y[i])<tol){
	Enew[j]+=E[i];
	znew[i]+=z[i]*E[i];
	found=1;
	break;
      }
    }
    if (!found){
      Enew.push_back(E[i]);
      xnew.push_back(x[i]);
      ynew.push_back(y[i]);
      znew.push_back(z[i]*E[i]);
    }
  }
  for(int j = 0; j<Enew.size();j++){
    znew[j]/=Enew[j];
  }
}


void recon_position(vector<double>& E, vector<double>& x, vector<double>& y, vector<double>& z, double & E_recon, double & x_recon,
		    double & y_recon, double & z_recon, position_recon_opts& opts){

  //first determine the total energy of the particle and of the shower
  E_recon=0;
  double E_shower=0;
  for (int i = 0; i<E.size();i++){
    E_recon += E[i]/opts.sampling_fraction;
    E_shower += E[i];
  }
  //now determine the value of w0 to use
  double w0=opts.w0_a+opts.w0_b*log(E_recon/opts.E0)+opts.w0_c*pow(log(E_recon/opts.E0),2);  
  //cout <<"w0" << w0 << endl;
  double sum_weights=0;
  //position of the cluster
  x_recon=0;y_recon=0;z_recon=0;
  for (int i=0; i<E.size(); i++){    
    double weight=max(log(E[i]/E_shower)+w0, 0.);
    sum_weights+=weight;
    x_recon+=x[i]*weight;
    y_recon+=y[i]*weight;
    z_recon+=z[i]*weight;
  }
  x_recon/=sum_weights;
  y_recon/=sum_weights;
  z_recon/=sum_weights;  
}
*/
