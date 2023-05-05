// Created by Simon Gardner to do LowQ2 pixel clustering
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <JANA/JEvent.h>
#include "LowQ2Cluster_factory.h"
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include "extensions/string/StringHelpers.h"
#include "ROOT/RVec.hxx"

namespace eicrecon {
  
  
  void LowQ2Cluster_factory::Init() {
    
    auto app = GetApplication();
    
    m_log = app->GetService<Log_service>()->logger(m_output_tag);
      
    
    m_geoSvc = app->GetService<JDD4hep_service>();
    
    
    
    m_log->info("RP Decoding complete...");
    
  }
  
  
  void LowQ2Cluster_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
    // Nothing to do here
  }
  
  void LowQ2Cluster_factory::Process(const std::shared_ptr<const JEvent> &event) {
    
    auto converter = m_geoSvc->cellIDPositionConverter();
    auto inputclusters = event->Get<eicrecon::TrackerProtoCluster>(m_input_tag);
    
    std::vector<TrackerClusterPoint*> outputClusterPoints;

    for(const auto protoCl: inputclusters ){
      
      auto clusterPoint = new eicrecon::TrackerClusterPoint();
      
      float esum = 0;
      float t0   = 0;
      float tE   = 0;

      ROOT::Math::XYZVector position(0,0,0);
      
      auto hits = *protoCl->associatedHits;

      for(auto hit : hits){
	auto hitE = hit.getCharge();
	auto hitT = hit.getTimeStamp();
	auto id   = hit.getCellID();

	if(!t0 || hitT<t0)
	  t0=hitT;
	esum += hitE;

	auto pos = converter->position(id);
	
	position+=ROOT::Math::XYZVector(pos.x(),pos.y(),pos.z())*hitE;

      }

      position/=esum;

      clusterPoint->chargeSum = esum;
      clusterPoint->time      = t0;
      clusterPoint->position  = edm4hep::Vector3f(position.x(),position.y(),position.z());

      clusterPoint->pCluster = protoCl;
      outputClusterPoints.push_back(clusterPoint);

    }
        
    Set(outputClusterPoints);
    
  }

}
