// Created by Alex Jentsch
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <JANA/JApplication.h>
#include <JANA/JEvent.h>

#include <extensions/jana/JChainFactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include "DD4hep/Detector.h"
#include <DDRec/CellIDPositionConverter.h>
#include <DDRec/Surface.h>
#include <DDRec/SurfaceManager.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <algorithms/fardetectors/MatrixTransferStatic.h>
#include <algorithms/fardetectors/MatrixTransferStaticConfig.h>

// Event Model related classes
#include <edm4eic/MutableReconstructedParticle.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/TrackerHit.h>
#include <edm4eic/vector_utils.h>
#include <edm4hep/SimTrackerHit.h>

#include "extensions/jana/JChainFactoryT.h"
#include "extensions/spdlog/SpdlogMixin.h"
#include <spdlog/logger.h>

namespace eicrecon {

    class MatrixTransferStatic_factory :
    public JChainFactoryT<edm4eic::ReconstructedParticle, MatrixTransferStaticConfig>{

    public:

        explicit MatrixTransferStatic_factory(const std::vector<std::string> default_input_tags, MatrixTransferStaticConfig cfg ):
	  JChainFactoryT(std::move(default_input_tags), cfg) {
	}

        /** One time initialization **/
        void Init() override;

        /** On run change preparations **/
        void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

    private:
	std::shared_ptr<spdlog::logger>  m_log;              // Logger for this factory
	eicrecon::MatrixTransferStatic   m_reco_algo;        // Actual algorithm
	std::shared_ptr<JDD4hep_service> m_geoSvc;
    };

} // eicrecon
