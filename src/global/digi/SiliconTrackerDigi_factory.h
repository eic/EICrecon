#pragma once

#include <random>

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <extensions/jana/JChainFactoryT.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <JANA/JEvent.h>

#include <edm4hep/SimTrackerHit.h>
#include <edm4hep/MCParticle.h>
#include <edm4hep/SimTrackerHitCollection.h>

#include <edm4eic/RawTrackerHit.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4eic/MutableRawTrackerHit.h>

#include <TGeoSystemOfUnits.h>
#include <TRandomGen.h>

#include <algorithms/digi/SiliconTrackerDigi.h>
#include <algorithms/digi/SiliconTrackerDigiConfig.h>

namespace eicrecon {

    class SiliconTrackerDigi;

    class SiliconTrackerDigi_factory :
            public JChainFactoryT<edm4eic::RawTrackerHit, SiliconTrackerDigiConfig>,
            public SpdlogMixin<SiliconTrackerDigi_factory> {

    public:

        explicit SiliconTrackerDigi_factory(const std::vector<std::string> &default_input_tags, SiliconTrackerDigiConfig cfg)
            : JChainFactoryT(default_input_tags, cfg) {}

        /** One time initialization **/
        void Init() override;

        /** On run change preparations **/
        void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

    private:

        eicrecon::SiliconTrackerDigi m_digi_algo;       /// Actual digitisation algorithm
    };

}

