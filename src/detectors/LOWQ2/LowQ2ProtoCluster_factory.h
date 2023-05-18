// Created by Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once
#include <spdlog/spdlog.h>

#include <services/geometry/dd4hep/JDD4hep_service.h>

// Event Model related classes
#include <edm4eic/TrackerHit.h>
#include <edm4eic/RawTrackerHit.h>

#include <extensions/jana/JChainFactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <spdlog/logger.h>

namespace eicrecon {

  struct TrackerProtoCluster {
    int layer;
    int module;
    std::vector<edm4eic::RawTrackerHit>* associatedHits;
  };

  class LowQ2ProtoCluster_factory : public JChainFactoryT<eicrecon::TrackerProtoCluster, NoConfig, JFactoryT>{

  public:

    LowQ2ProtoCluster_factory( std::vector<std::string> default_input_tags):
      JChainFactoryT<eicrecon::TrackerProtoCluster, NoConfig, JFactoryT>(std::move(default_input_tags) ) {
    }


      /** One time initialization **/
      void Init() override;

      /** On run change preparations **/
      void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

      /** Event by event processing **/
      void Process(const std::shared_ptr<const JEvent> &event) override;

      //----- Define constants here ------
      std::string m_readout{"TaggerTrackerHits"};
      std::string m_moduleField{"module"};
      std::string m_layerField{"layer"};
      std::string m_xField{"x"};
      std::string m_yField{"y"};

      dd4hep::BitFieldCoder *id_dec{nullptr};
      size_t module_idx{0}, layer_idx{0}, x_idx{0}, y_idx{0};

      std::shared_ptr<JDD4hep_service> m_geoSvc;


  private:
      std::shared_ptr<spdlog::logger> m_log;              /// Logger for this factory
	std::string m_input_tag {"TaggerTrackerRawHit"};
	std::string m_output_tag{"TaggerTrackerProtoClusters"};

  };

} // eicrecon
