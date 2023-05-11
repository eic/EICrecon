// Created by Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <spdlog/spdlog.h>

#include <DDRec/CellIDPositionConverter.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>

// Event Model related classes
#include "LowQ2ProtoCluster_factory.h"

#include <extensions/jana/JChainFactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <spdlog/logger.h>


namespace eicrecon {

    struct TrackerClusterPoint {
      float chargeSum{0};    
      float time{0};
      float timeError{0};
      edm4hep::Vector3f position;
      edm4eic::CovDiag3f positionError;
      const eicrecon::TrackerProtoCluster* pCluster;
    };

    class LowQ2Cluster_factory : public JChainFactoryT<eicrecon::TrackerClusterPoint, NoConfig, JFactoryT>{

    public:
    
        LowQ2Cluster_factory( std::vector<std::string> default_input_tags):
                JChainFactoryT<eicrecon::TrackerClusterPoint, NoConfig, JFactoryT>(std::move(default_input_tags) ) {
        }

        LowQ2Cluster_factory(); //constructer

        /** One time initialization **/
        void Init() override;

        /** On run change preparations **/
        void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

	//----- Define constants here ------
	std::shared_ptr<JDD4hep_service> m_geoSvc;

	private:
	  std::shared_ptr<spdlog::logger> m_log;              /// Logger for this factory
	  std::string m_input_tag{"TaggerTrackerProtoClusters"};
	  std::string m_output_tag{"TaggerTrackerClusterPositions"};

    };

} // eicrecon
