// Created by Simon Gardner to do LowQ2 pixel clustering
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <JANA/JEvent.h>
#include "LowQ2ProtoCluster_factory.h"
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include "extensions/string/StringHelpers.h"
#include "ROOT/RVec.hxx"

namespace eicrecon {
  
  
  void LowQ2ProtoCluster_factory::Init() {
    
    auto app = GetApplication();
    
    m_log = app->GetService<Log_service>()->logger(m_output_tag);
    
    m_readout     = "TaggerTrackerHits";
    m_moduleField = "module";
    m_layerField  = "layer";
    m_xField      = "x";
    m_yField      = "y";
    
    
    m_geoSvc = app->GetService<JDD4hep_service>();
    
    if(m_readout.empty()){ m_log->error("READOUT IS EMPTY!"); return; }
    
    auto id_spec = m_geoSvc->detector()->readout(m_readout).idSpec();
    try {
      id_dec = id_spec.decoder();
      if (!m_moduleField.empty()) {
	module_idx = id_dec->index(m_moduleField);
	m_log->info("Find module field {}, index = {}", m_moduleField, module_idx);
      }
      if (!m_layerField.empty()) {
	layer_idx = id_dec->index(m_layerField);
	m_log->info("Find layer field {}, index = {}", m_layerField, layer_idx);
      }
      if (!m_xField.empty()) {
	x_idx = id_dec->index(m_xField);
	m_log->info("Find layer field {}, index = {}",  m_xField,      x_idx);
      }
      if (!m_yField.empty()) {
	y_idx = id_dec->index(m_yField);
	m_log->info("Find layer field {}, index = {}", m_yField, y_idx);
      }
    } catch (...) {
      m_log->error("Failed to load ID decoder for {}", m_readout);
      throw JException("Failed to load ID decoder");
    }
    
    
    m_log->info("RP Decoding complete...");
    
  }
  
  
  void LowQ2ProtoCluster_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
    // Nothing to do here
  }
  
  void LowQ2ProtoCluster_factory::Process(const std::shared_ptr<const JEvent> &event) {
    
    auto converter = m_geoSvc->cellIDPositionConverter();
    auto inputhits = event->Get<edm4eic::RawTrackerHit>(m_input_tag);
        
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
    ROOT::VecOps::RVec<bool> avaliable(module.size(),1);
    //ROOT::VecOps::RVec<int>  clusterNo(module.size(),0);
    ROOT::VecOps::RVec<int>  indeces  (module.size());

    std::vector<TrackerProtoCluster*> outputProtoClusters;

    for(ulong i = 0; i<indeces.size(); i++)
      indeces[i] = i;
   
    //int seedNumber = 0;

    while(ROOT::VecOps::Any(avaliable)){

      auto pCluster = new eicrecon::TrackerProtoCluster();
      pCluster->associatedHits = new std::vector<edm4eic::RawTrackerHit>;
      //eicrecon::TrackerProtoCluster pCluster;
      
      //Cluster seed
      auto maxIndex = ROOT::VecOps::ArgMax(e*avaliable);

      avaliable[maxIndex] = 0;
      //clusterNo[maxIndex] = seedNumber;

      ROOT::VecOps::RVec<ulong> indexList = {maxIndex};
      
      pCluster->module = module[maxIndex];
      pCluster->layer  = layer[maxIndex];

      while(indexList.size()){
	
	auto index = indexList[0];
	auto filter = avaliable*(module==module[index])*(layer==layer[index])*(abs(x-x[index])<=1)*(abs(y-y[index])<=1)*(abs(t-t[index])<1);
	avaliable = avaliable*(!filter);
	//clusterNo += filter*seedNumber;
	indexList = Concatenate(indexList,indeces[filter]);
	
	indexList.erase(indexList.begin());

	pCluster->associatedHits->push_back(*inputhits[index]);
	
      }
      //seedNumber++;

      //std::cout << pCluster->associatedHits->size() << std::endl;

      outputProtoClusters.push_back(pCluster);

    }    
    
    Set(outputProtoClusters);
    
  }

}
