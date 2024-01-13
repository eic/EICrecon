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

  double sl=m_cfg.side_length/dd4hep::mm;
  double layer_spacing=m_cfg.layer_spacing/dd4hep::mm;
  double MIP=m_cfg.MIP/dd4hep::GeV;
  double Emin=m_cfg.Emin_in_MIPs*MIP;
  double tmax=m_cfg.tmax/dd4hep::ns;

  auto volman = m_detector->volumeManager();
  auto subcellHits = std::make_unique<edm4eic::CalorimeterHitCollection>();
  double Esum=0;
  for(const auto& hit : hits){
    //skip hits that do not pass E and t cuts
    if (hit.getEnergy()<Emin || hit.getTime()>tmax)
      continue;
    //keep track of the energy in each neighboring cell
    std::vector<double> Eneighbors(SUBCELLS, 0.0);
    
    for (const auto& other_hit : hits){                                                                                            
      double tol=0.01; //tolerance for rounding errors
      
      //only look at hits nearby within two layers of the current layer
      double dz=abs(hit.getLocal().z-other_hit.getLocal().z);
      if (dz>2.5*layer_spacing || dz<tol)
        continue;
      if (other_hit.getEnergy()<Emin || other_hit.getTime()>tmax)
        continue;
      //difference in transverse position (in units of side lengths)
      double dx=(other_hit.getLocal().x-hit.getLocal().x)/sl;
      double dy=(other_hit.getLocal().y-hit.getLocal().y)/sl;
      if (abs(dx)>2 || abs(dy)>sqrt(3))
        continue;

      //loop over locations of the neighboring cells
      //and check if the jth hit matches this location
      for(int k=0;k<SUBCELLS;k++){
        if(abs(dx-neighbor_offsets_x[k])<tol && abs(dy-neighbor_offsets_y[k])<tol){
          Eneighbors[k]+=other_hit.getEnergy();
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
      const decltype(edm4eic::CalorimeterHitData::local) local(hit.getLocal().x+subcell_offsets_x[k]*sl, hit.getLocal().y+subcell_offsets_y[k]*sl, hit.getLocal().z);

      //convert this to a position object so that the global position can be determined
      dd4hep::Position local_position;
      local_position.SetX(local.x);
      local_position.SetY(local.y);
      local_position.SetZ(local.z);

      //also convert this to the detector's global coordinates.  To do: check if this is correct
      auto alignment = volman.lookupDetElement(hit.getCellID()).nominal();

      auto global_position = alignment.localToWorld(local_position);
      //convert this from position object to a vector object
      const decltype(edm4eic::CalorimeterHitData::position) position = {global_position.X(), global_position.Y(), global_position.Z()};

      //bounding box dimensions depend on the orientation of the rhombus
      int orientation = k%3==0;
      const decltype(edm4eic::CalorimeterHitData::dimension) dimension(sl*(orientation?1:1.5), sl*sqrt(3)/2.*(orientation?2:1),
                                                                       hit.getDimension()[2]);

      subcellHits->create(
            hit.getCellID(),
            hit.getEnergy()*weights[k]/sum_weights,
            0,
            hit.getTime(),
            0,
            position,
            dimension,
            hit.getSector(),
            hit.getLayer(),
            local);
    }
  }
  return subcellHits;
}


} // namespace eicrecon
