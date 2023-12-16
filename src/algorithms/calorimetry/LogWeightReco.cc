// Copyright (C) 2023 Sebouh Paul
// SPDX-License-Identifier: LGPL-3.0-or-later

// References:
//   https://arxiv.org/abs/2308.06939


#include <DD4hep/Readout.h>
#include <Evaluator/DD4hepUnits.h>
#include <TInterpreter.h>
#include <TInterpreterValue.h>
#include <edm4hep/Vector3f.h>
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

#include "LogWeightReco.h"

#include "algorithms/calorimetry/LogWeightRecoConfig.h"

//using namespace edm4eic;

namespace eicrecon {

  void LogWeightReco::init(const dd4hep::Detector* detector, std::shared_ptr<spdlog::logger>& logger) {
    m_log = logger;
    m_detector = detector;
  }

  std::unique_ptr<edm4eic::ClusterCollection>
    LogWeightReco::process(const edm4eic::ProtoClusterCollection &proto) {  
    
    auto outputClusters = std::make_unique<edm4eic::ClusterCollection>();
    for (const auto& pcl : proto)
    {
      if (pcl.hits_size() == 0)
        continue;

      const auto& hits = pcl.getHits();
      //first determine the total energy of the particle and of the shower
      double E_recon=0;
      double E_shower=0;
      for (int i=0; i<hits.size(); i++) {
        E_shower += hits[i].getEnergy();
      }
      E_recon += E_shower/m_cfg.sampling_fraction;

      //now determine the value of w0 to use
      double w0=m_cfg.w0_a+m_cfg.w0_b*log(E_recon/m_cfg.E0)+m_cfg.w0_c*pow(log(E_recon/m_cfg.E0),2);  
      m_log->debug("w0: {}", w0);
      double sum_weights=0;
      //position of the cluster
      double x_recon=0, y_recon=0, z_recon=0, t_recon=0;
      for (int i=0; i<hits.size(); i++){    
        double weight=std::max(log(hits[i].getEnergy()/E_shower)+w0, 0.);
        sum_weights+=weight;
        x_recon+=hits[i].getPosition().x*weight;
        y_recon+=hits[i].getPosition().y*weight;
        z_recon+=hits[i].getPosition().z*weight;
        t_recon+=hits[i].getTime()*weight;
      }
      if (sum_weights == 0)
      {
        m_log->warn("0 weight");
        continue;
      }
      x_recon/=sum_weights;
      y_recon/=sum_weights;
      z_recon/=sum_weights;
      t_recon/=sum_weights;

      edm4hep::Vector3f position(x_recon, y_recon, z_recon);

      auto clus = outputClusters->create();
      clus.setNhits(hits.size());
      clus.setEnergy(E_recon);
      clus.setTime(t_recon);
      clus.setPosition(position);
    }

    return outputClusters;
  }

}
