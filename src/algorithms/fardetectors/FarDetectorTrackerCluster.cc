// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Simon Gardner

#include <edm4hep/TrackerHit.h>
#include <edm4eic/RawTrackerHit.h>

#include <ROOT/RVec.hxx>

#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include "FarDetectorTrackerCluster.h"

namespace eicrecon {


  void FarDetectorTrackerCluster::init(const dd4hep::Detector* det,
	      std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> cellid,
              std::shared_ptr<spdlog::logger> log) {

    m_log = log;
    m_detector = det;
    m_cellid_converter = cellid;

    if (m_cfg.readout.empty()) {
      throw JException("Readout is empty");
    }
    
    try {
      m_id_dec = m_detector->readout(m_cfg.readout).idSpec().decoder();
      if (!m_cfg.moduleField.empty()) {
	m_module_idx = m_id_dec->index(m_cfg.moduleField);
	m_log->debug("Find module field {}, index = {}", m_cfg.moduleField, m_module_idx);
      }
      if (!m_cfg.layerField.empty()) {
	m_layer_idx = m_id_dec->index(m_cfg.layerField);
	m_log->debug("Find layer field {}, index = {}", m_cfg.layerField, m_layer_idx);
      }
      if (!m_cfg.xField.empty()) {
	m_x_idx = m_id_dec->index(m_cfg.xField);
	m_log->debug("Find layer field {}, index = {}",  m_cfg.xField, m_x_idx);
      }
      if (!m_cfg.yField.empty()) {
	m_y_idx = m_id_dec->index(m_cfg.yField);
	m_log->debug("Find layer field {}, index = {}", m_cfg.yField, m_y_idx);
      }
    } catch (...) {
      m_log->error("Failed to load ID decoder for {}", m_cfg.readout);
      throw JException("Failed to load ID decoder");
    }
    
    

  }

  std::unique_ptr<edm4hep::TrackerHitCollection> FarDetectorTrackerCluster::produce(const edm4eic::RawTrackerHitCollection &inputhits) {
    // TODO check if this whole method is unnecessarily complicated/inefficient

    ROOT::VecOps::RVec<long>  id;
    ROOT::VecOps::RVec<int>   module;
    ROOT::VecOps::RVec<int>   layer;
    ROOT::VecOps::RVec<int>   x;
    ROOT::VecOps::RVec<int>   y;
    ROOT::VecOps::RVec<float> e;
    ROOT::VecOps::RVec<float> t;

    // Gather detector id positions
    for(auto hit: inputhits){
      auto cellID = hit.getCellID();
      id.push_back    (cellID);
      module.push_back(m_id_dec->get( cellID, m_module_idx ));
      layer.push_back (m_id_dec->get( cellID, m_layer_idx  ));
      x.push_back     (m_id_dec->get( cellID, m_x_idx      ));
      y.push_back     (m_id_dec->get( cellID, m_y_idx      ));
      e.push_back     (hit.getCharge());
      t.push_back     (hit.getTimeStamp());
    }

    // Set up clustering variables
    ROOT::VecOps::RVec<bool> available(module.size(), 1);
    auto indices = Enumerate(module);

    auto outputClusters = std::make_unique<edm4hep::TrackerHitCollection>();

    // Loop while there are unclustered hits
    while(ROOT::VecOps::Any(available)){

      auto cluster = outputClusters->create();

      double xPos      = 0;
      double yPos      = 0;
      double zPos      = 0;
      float  weightSum = 0;

      float esum   = 0;
      float t0     = 0;
      float tError = 0;

      auto maxIndex = ROOT::VecOps::ArgMax(e*available);

      available[maxIndex] = 0;

      ROOT::VecOps::RVec<ulong> clusterList = {maxIndex};
      ROOT::VecOps::RVec<float> clusterT;

      // Filter to make sure everything is on the same detector layer
      auto layerFilter = (module==module[maxIndex])*(layer==layer[maxIndex]);

      // Loop over hits, adding neighbouring hits as relevant
      while(clusterList.size()){

	// Takes first remaining hit in cluster list
        auto index  = clusterList[0];

	// Finds neighbours of cluster within time limit
        auto filter = available*layerFilter*(abs(x-x[index])<=1)*(abs(y-y[index])<=1)*(abs(t-t[index])<m_cfg.time_limit);
	
	// Adds the found hits to the cluster
        clusterList = Concatenate(clusterList,indices[filter]);

	// Removes the found hits from the list of still avaliable hits
        available = available*(!filter);

	// Removes current hit from remaining found cluster hits 
        clusterList.erase(clusterList.begin());
	
	// Adds raw hit to TrackerHit contribution
        cluster.addToRawHits(inputhits[index].getObjectID());

        //Energy
        auto hitE = e[index];
        esum += hitE;
        auto pos = m_cellid_converter->position(id[index]);

        //Weighted position
        float weight = hitE; // TODO - Calculate appropriate weighting based on sensor charge sharing
        weightSum += weight;
        xPos += pos.x()*weight;
        yPos += pos.y()*weight;
        zPos += pos.z()*weight;

        //Time
        clusterT.push_back(t[index]);


      }

      // Finalise position
      xPos/=weightSum;
      yPos/=weightSum;
      zPos/=weightSum;

      // Finalise time
      t0      = Mean(clusterT);
      tError  = StdDev(clusterT);

      // Set cluster members
      cluster.setCellID  (id[maxIndex]);
      cluster.setPosition(edm4hep::Vector3d(xPos,yPos,zPos));
      cluster.setEDep    (esum);
      cluster.setTime    (t0);

    }

    return outputClusters;

  }

}
