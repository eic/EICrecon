#pragma once

#include <random>

#include <spdlog/spdlog.h>


#include <edm4hep/MCParticle.h>
#include <edm4hep/SimTrackerHitCollection.h>

#include <edm4eic/RawTrackerHit.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4eic/MutableRawTrackerHit.h>

#include <edm4eic/MutableTrackerHit.h>

#include <algorithms/tracking/TrackerHitReconstructionConfig.h>
#include <algorithms/tracking/TrackerHitReconstruction.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <extensions/jana/JChainFactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>


class TrackerHitReconstruction_factory :
        public JChainFactoryT<edm4eic::TrackerHit, eicrecon::TrackerHitReconstructionConfig>,
        public eicrecon::SpdlogMixin<TrackerHitReconstruction_factory> {

public:
    TrackerHitReconstruction_factory( std::vector<std::string> default_input_tags, eicrecon::TrackerHitReconstructionConfig cfg):
            JChainFactoryT<edm4eic::TrackerHit, eicrecon::TrackerHitReconstructionConfig>( std::move(default_input_tags), cfg ) {
    }

    /** One time initialization **/
    void Init() override;

    /** On run change preparations **/
    void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

    /** Event by event processing **/
    void Process(const std::shared_ptr<const JEvent> &event) override;

private:

    eicrecon::TrackerHitReconstruction m_reco_algo;     /// The reconstruction algorithm
};
