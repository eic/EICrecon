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

std::unique_ptr<edm4eic::CalorimeterHitCollection> HEXPLIT::process(const edm4eic::CalorimeterHitCollection &hits){
  int nhits=hits.size();
  double sl=m_cfg.side_length/dd4hep::mm;
  double layer_spacing=m_cfg.layer_spacing/dd4hep::mm;
  double MIP=m_cfg.MIP/dd4hep::GeV;
  double Emin=m_cfg.Emin_in_MIPs*MIP/dd4hep::GeV;
  double tmax=m_cfg.tmax/dd4hep::ns;


  /*double x[nhits];
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
    }*/
  auto volman = m_detector->volumeManager();
  auto subcellHits = std::make_unique<edm4eic::CalorimeterHitCollection>();
  double Esum=0;
  for(int i=0; i<nhits; i++){
    //skip hits that do not pass E and t cuts
    if (hits[i].getEnergy()<Emin || hits[i].getTime()>tmax)
      continue;
    //keep track of the energy in each neighboring cell
    double Eneighbors[SUBCELLS];
    for (int j=0; j<SUBCELLS; j++)
      Eneighbors[j]=0;

    for (int j=0; j<nhits; j++){
      //only look at hits nearby within two layers of the current layer
      double dz=abs(hits[i].getLocal().z-hits[j].getLocal().z);
      if (dz>2.5*layer_spacing || dz==0)
        continue;
      if (hits[j].getEnergy()<Emin || hits[j].getTime()>tmax)
        continue;
      //difference in transverse position (in units of side lengths)
      double dx=(hits[j].getLocal().x-hits[i].getLocal().x)/sl;
      double dy=(hits[j].getLocal().y-hits[i].getLocal().y)/sl;
      if (abs(dx)>2 || abs(dy)>sqrt(3))
        continue;

      //loop over locations of the neighboring cells
      //and check if the jth hit matches this location
      double tol=0.01; //tolerance for rounding errors
      for(int k=0;k<SUBCELLS;k++){
        if(abs(dx-neighbor_offsets_x[k])<tol && abs(dy-neighbor_offsets_y[k])<tol){
          Eneighbors[k]+=hits[j].getEnergy();
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

      //create the subcell hits.  First determine their positions in local coordinates.
      const decltype(edm4eic::CalorimeterHitData::local) local(hits[i].getLocal().x+subcell_offsets_x[k]*sl, hits[i].getLocal().y+subcell_offsets_y[k]*sl, hits[i].getLocal().z);

      //convert this to a position object so that the global position can be determined
      dd4hep::Position local_position;
      local_position.SetX(local.x);
      local_position.SetY(local.y);
      local_position.SetZ(local.z);

      //also convert this to the detector's global coordinates.  To do: check if this is correct
      auto alignment = volman.lookupDetElement(hits[i].getCellID()).nominal();

      auto global_position = alignment.localToWorld(local_position);
      //convert this from position object to a vector object
      const decltype(edm4eic::CalorimeterHitData::position) position = {global_position.X(), global_position.Y(), global_position.Z()};

      //bounding box dimensions depend on the orientation of the rhombus
      int orientation = k%3==0;
      const decltype(edm4eic::CalorimeterHitData::dimension) dimension(sl*(orientation?1:1.5), sl*sqrt(3)/2.*(orientation?2:1),
                                                                       hits[i].getDimension()[2]);

      subcellHits->create(
            hits[i].getCellID(),
            hits[i].getEnergy()*weights[k]/sum_weights,
            0,
            hits[i].getTime(),
            0,
            position,
            dimension,
            hits[i].getSector(),
            hits[i].getLayer(),
            local);
    }
  }
  return subcellHits;
}


} // namespace eicrecon
