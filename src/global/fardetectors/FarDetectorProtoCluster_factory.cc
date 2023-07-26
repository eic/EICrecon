// Created by Simon Gardner to do LowQ2 pixel clustering
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <JANA/JEvent.h>
#include <edm4eic/TrackerHit.h>
#include <edm4eic/RawTrackerHit.h>

#include "FarDetectorProtoCluster_factory.h"
#include "services/log/Log_service.h"
#include "extensions/spdlog/SpdlogExtensions.h"
#include "ROOT/RVec.hxx"

namespace eicrecon {


  void FarDetectorProtoCluster_factory::Init() {

    auto app = GetApplication();

    m_log = app->GetService<Log_service>()->logger(GetOutputTags()[0]);


  }


  void FarDetectorProtoCluster_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
    // Nothing to do here
  }

  void FarDetectorProtoCluster_factory::Process(const std::shared_ptr<const JEvent> &event) {
    // TODO check if this whole method is unnecessarily complicated/inefficient

    auto inputhits  = event->Get<edm4eic::RawTrackerHit>(GetInputTags()[0]);

    auto outputhits = m_reco_algo->Process(inputhits);

    Set(std::move(outputProtoClusters));

  }

}
