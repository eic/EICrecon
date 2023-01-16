// Created by Christopher Dilks
// Based on SiliconTrackerDigi_factory
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

// JANA
#include <extensions/jana/JChainFactoryT.h>
#include <JANA/JEvent.h>

// data model
#include <edm4hep/SimTrackerHit.h>
#include <edm4eic/RawPMTHit.h>

// algorithms
#include <algorithms/digi/PhotoMultiplierHitDigi.h>
#include <algorithms/digi/PhotoMultiplierHitDigiConfig.h>

// services
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <extensions/string/StringHelpers.h>

namespace eicrecon {

    class PhotoMultiplierHitDigi;

    class PhotoMultiplierHitDigi_factory :
            public JChainFactoryT<edm4eic::RawPMTHit, PhotoMultiplierHitDigiConfig>,
            public SpdlogMixin<PhotoMultiplierHitDigi_factory> {

    public:

        explicit PhotoMultiplierHitDigi_factory(std::vector<std::string> default_input_tags, PhotoMultiplierHitDigiConfig cfg) :
            JChainFactoryT<edm4eic::RawPMTHit, PhotoMultiplierHitDigiConfig>(std::move(default_input_tags), cfg) {}

        /** One time initialization **/
        void Init() override;

        /** On run change preparations **/
        void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

    private:

        eicrecon::PhotoMultiplierHitDigi m_digi_algo;       /// Actual digitisation algorithm
    };

}
