// Created by Alex Jentsch
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include <extensions/spdlog/SpdlogMixin.h>
#include <DDRec/CellIDPositionConverter.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <algorithms/fardetectors/MatrixTransferStatic.h>
#include <algorithms/fardetectors/MatrixTransferStaticConfig.h>

// Event Model related classes
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>

#include "extensions/jana/JChainMultifactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"
#include <spdlog/logger.h>

namespace eicrecon {

    class MatrixTransferStatic_factoryT :
    public JChainMultifactoryT<MatrixTransferStaticConfig>{

    public:

        explicit MatrixTransferStatic_factoryT(
	  std::string tag,
          const std::vector<std::string>& input_tags,
          const std::vector<std::string>& output_tags,
          MatrixTransferStaticConfig cfg ):
	  JChainMultifactoryT<MatrixTransferStaticConfig>(std::move(tag), input_tags, output_tags, cfg) {

	  DeclarePodioOutput<edm4eic::ReconstructedParticle>(GetOutputTags()[0]);

	}

        /** One time initialization **/
	void Init() override {

	  auto app = GetApplication();

	  m_log = app->GetService<Log_service>()->logger(GetTag());
	  
	  auto cfg = GetDefaultConfig();
	  
	  m_geoSvc = app->GetService<JDD4hep_service>();
	  
	  m_reco_algo.setDetector(m_geoSvc->detector());
	  m_reco_algo.setGeoConverter(m_geoSvc->cellIDPositionConverter());
	  m_reco_algo.applyConfig(cfg);
	  m_reco_algo.init(m_log);

        }

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override {

	  auto inputhits = static_cast<const edm4hep::SimTrackerHitCollection*>(event->GetCollectionBase(GetInputTags()[0]));
	  
	  try {
	    auto outputTracks = m_reco_algo.produce(*inputhits);
	    SetCollection<edm4eic::ReconstructedParticle>(GetOutputTags()[0], std::move(outputTracks));
	  }
	  catch(std::exception &e) {
	    throw JException(e.what());
	  }


	}

    private:
	std::shared_ptr<spdlog::logger>  m_log;              // Logger for this factory
	eicrecon::MatrixTransferStatic   m_reco_algo;        // Actual algorithm
	std::shared_ptr<JDD4hep_service> m_geoSvc;
    };

} // eicrecon
