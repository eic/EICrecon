// Created by Simon Gardner to do LowQ2 pixel clustering
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <edm4eic/TrackerHit.h>
#include <edm4eic/RawTrackerHit.h>

#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include "ROOT/RVec.hxx"

namespace eicrecon {


  void FarDetectorProtoCluster_factory::Init() {

    auto app = GetApplication();

    m_log = app->GetService<Log_service>()->logger(m_output_tag);

    m_geoSvc = app->GetService<JDD4hep_service>();

    if (m_readout.empty()) {
      throw JException("Readout is empty");
    }

    try {
      id_dec = m_geoSvc->detector()->readout(m_readout).idSpec().decoder();
      if (!m_moduleField.empty()) {
	module_idx = id_dec->index(m_moduleField);
	m_log->debug("Find module field {}, index = {}", m_moduleField, module_idx);
      }
      if (!m_layerField.empty()) {
	layer_idx = id_dec->index(m_layerField);
	m_log->debug("Find layer field {}, index = {}", m_layerField, layer_idx);
      }
      if (!m_xField.empty()) {
	x_idx = id_dec->index(m_xField);
	m_log->debug("Find layer field {}, index = {}",  m_xField, x_idx);
      }
      if (!m_yField.empty()) {
	y_idx = id_dec->index(m_yField);
	m_log->debug("Find layer field {}, index = {}", m_yField, y_idx);
      }
    } catch (...) {
      m_log->error("Failed to load ID decoder for {}", m_readout);
      throw JException("Failed to load ID decoder");
    }


  }

  std::vector<eicrecon::TrackerProtoCluster*> TrackerProtoCluster::produce(const edm4eic::RawTrackerHitCollection &inputhits) {
    // TODO check if this whole method is unnecessarily complicated/inefficient

    ROOT::VecOps::RVec<int>   module;
    ROOT::VecOps::RVec<int>   layer;
    ROOT::VecOps::RVec<int>   x;
    ROOT::VecOps::RVec<int>   y;
    ROOT::VecOps::RVec<int>   e;
    ROOT::VecOps::RVec<int>   t;

    // Gather detector id positions
    for(auto hit: inputhits){
      auto cellID = hit->getCellID();
      module.push_back(id_dec->get( cellID, module_idx ));
      layer.push_back (id_dec->get( cellID, layer_idx  ));
      x.push_back     (id_dec->get( cellID, x_idx      ));
      y.push_back     (id_dec->get( cellID, y_idx      ));
      e.push_back     (hit->getCharge());
      t.push_back     (hit->getTimeStamp());
    }

    // Set up clustering variables
    ROOT::VecOps::RVec<bool> available(module.size(), 1);
    ROOT::VecOps::RVec<int>  indices  (module.size());

    std::vector<eicrecon::TrackerProtoCluster*> outputProtoClusters;

    for(ulong i = 0; i<indices.size(); i++)
      indices[i] = i;

    // Loop while there are unclustered hits
    while(ROOT::VecOps::Any(available)){

      auto pCluster = new eicrecon::TrackerProtoCluster();
      //pCluster->associatedHits = std::vector<edm4eic::RawTrackerHit>;

      auto maxIndex = ROOT::VecOps::ArgMax(e*available);

      available[maxIndex] = 0;

      ROOT::VecOps::RVec<ulong> indexList = {maxIndex};

      pCluster->module = module[maxIndex];
      pCluster->layer  = layer[maxIndex];

      // Filter to make sure everything is on the same detector layer
      auto layerFilter = (module==module[maxIndex])*(layer==layer[maxIndex]);

      // Loop over hits, adding neighbouring hits as relevant
      while(indexList.size()){

	auto index = indexList[0];
	auto filter = available*layerFilter*(abs(x-x[index])<=1)*(abs(y-y[index])<=1)*(abs(t-t[index])<1);
	available = available*(!filter);
	indexList = Concatenate(indexList,indices[filter]);

	indexList.erase(indexList.begin());

	pCluster->associatedHits.push_back(*inputhits[index]);

      }

      outputProtoClusters.push_back(pCluster);

    }

    return outputProtoClusters;

  }

}
