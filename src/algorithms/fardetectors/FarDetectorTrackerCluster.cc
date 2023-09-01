// Created by Simon Gardner to do LowQ2 pixel clustering
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <edm4hep/TrackerHit.h>
#include <edm4eic/RawTrackerHit.h>

#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include "ROOT/RVec.hxx"
#include "FarDetectorTrackerCluster.h"

namespace eicrecon {


  void FarDetectorTrackerCluster::init(std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> cellid,
	      dd4hep::BitFieldCoder* id_dec,
	      std::shared_ptr<spdlog::logger> log) {
    m_log = log;
    m_id_dec = id_dec;
    m_cellid_converter = cellid;
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
      module.push_back(m_id_dec->get( cellID, m_cfg.module_idx ));
      layer.push_back (m_id_dec->get( cellID, m_cfg.layer_idx  ));
      x.push_back     (m_id_dec->get( cellID, m_cfg.x_idx      ));
      y.push_back     (m_id_dec->get( cellID, m_cfg.y_idx      ));
      e.push_back     (hit.getCharge());
      t.push_back     (hit.getTimeStamp());
    }

    // Set up clustering variables
    ROOT::VecOps::RVec<bool> available(module.size(), 1);
    ROOT::VecOps::RVec<int>  indices  (module.size());

//     std::vector<eicrecon::TrackerCluster*> outputClusters;
    auto outputClusters = std::make_unique<edm4hep::TrackerHitCollection>();

    for(ulong i = 0; i<indices.size(); i++)
      indices[i] = i;

    // Loop while there are unclustered hits
    while(ROOT::VecOps::Any(available)){

      //      auto cluster = new edm4hep::MutableTrackerHit();
      edm4hep::MutableTrackerHit cluster;

      double xPos      = 0;
      double yPos      = 0;
      double zPos      = 0;
      float  weightSum = 0;

      float esum   = 0;
      float t0     = 0;
      float tError = 0;
      //pCluster->associatedHits = std::vector<edm4eic::RawTrackerHit>;

      auto maxIndex = ROOT::VecOps::ArgMax(e*available);

      available[maxIndex] = 0;

      ROOT::VecOps::RVec<ulong> indexList = {maxIndex};
      ROOT::VecOps::RVec<float> clusterT;

//       pCluster->module = module[maxIndex];
//       pCluster->layer  = layer[maxIndex];

      // Filter to make sure everything is on the same detector layer
      auto layerFilter = (module==module[maxIndex])*(layer==layer[maxIndex]);

      // Loop over hits, adding neighbouring hits as relevant
      while(indexList.size()){

	auto index = indexList[0];
	auto filter = available*layerFilter*(abs(x-x[index])<=1)*(abs(y-y[index])<=1)*(abs(t-t[index])<1);
	available = available*(!filter);
	indexList = Concatenate(indexList,indices[filter]);

	indexList.erase(indexList.begin());

	cluster.addToRawHits(inputhits[index].getObjectID());

	//Energy
	auto hitE = e[index];
	//auto id   = hit.getCellID();
	esum += hitE;
	auto pos = m_cellid_converter->position(id[index]);
// 	std::cout << pos << std::endl;
	//Weighted position
	float weight = hitE; //Check appropriate weighting
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
//       xPos*=10;
//       yPos*=10;
//       zPos*=10;

//       std::cout << xPos << " " << yPos << " " << zPos << std::endl << std::endl;

      // Finalise time
      t0      = Mean(clusterT);
      tError  = StdDev(clusterT);

      cluster.setCellID  (id[maxIndex]);
      cluster.setPosition(edm4hep::Vector3d(xPos,yPos,zPos));
      cluster.setEDep    (esum);
      cluster.setTime    (t0);

      outputClusters->push_back(edm4hep::TrackerHit(cluster));

    }

    return outputClusters;

  }

}
