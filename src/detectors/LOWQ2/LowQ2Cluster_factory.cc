// Created by Simon Gardner to do LowQ2 pixel clustering
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <JANA/JEvent.h>
#include <edm4eic/TrackerHit.h>

#include "LowQ2Cluster_factory.h"
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include "ROOT/RVec.hxx"

namespace eicrecon {


  void LowQ2Cluster_factory::Init() {

    auto app = GetApplication();

    m_log = app->GetService<Log_service>()->logger(m_output_tag);

    m_geoSvc  = app->GetService<JDD4hep_service>();

  }


  void LowQ2Cluster_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
    // Nothing to do here
  }

  void LowQ2Cluster_factory::Process(const std::shared_ptr<const JEvent> &event) {

    auto inputclusters = event->Get<eicrecon::TrackerProtoCluster>(m_input_tag);
    auto cellid_converter = m_geoSvc->cellIDPositionConverter();

    std::vector<TrackerClusterPoint*> outputClusterPoints(inputclusters.size());

    // Find the energy weighted center of each protocluster
    int iclust = 0;
    for(const auto protoCl: inputclusters ){

      float esum = 0;
      float t0   = 0;
      float tE   = 0;

      ROOT::Math::XYZVector position(0,0,0);

      auto hits = protoCl->associatedHits;

      // Loop over hits contributing to the protocluster
      for (const auto &hit : hits) {
	auto hitE = hit.getCharge();
	auto hitT = hit.getTimeStamp();
	auto id   = hit.getCellID();

	if(!t0 || hitT<t0)
	  t0=hitT;
	esum += hitE;

	auto pos = cellid_converter->position(id);

	position+=ROOT::Math::XYZVector(pos.x(),pos.y(),pos.z())*hitE;

      }

      position/=esum;

      auto clusterPoint = new eicrecon::TrackerClusterPoint();
      clusterPoint->chargeSum = esum;
      clusterPoint->time      = t0;
      clusterPoint->position  = edm4hep::Vector3f(position.x(),position.y(),position.z());

      clusterPoint->pCluster = protoCl;
      outputClusterPoints[iclust++] = clusterPoint;

    }

    Set(std::move(outputClusterPoints));

  }

}
