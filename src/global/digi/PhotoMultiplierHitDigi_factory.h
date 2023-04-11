// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Christopher Dilks

#pragma once

// JANA
#include <extensions/jana/JChainFactoryT.h>
#include <JANA/JEvent.h>

// data model
#include <edm4hep/SimTrackerHit.h>
#include <edm4eic/MCRecoTrackerHitAssociation.h>

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
            public JChainFactoryT<edm4eic::MCRecoTrackerHitAssociation, PhotoMultiplierHitDigiConfig>,
            public SpdlogMixin<PhotoMultiplierHitDigi_factory> {

    public:

        explicit PhotoMultiplierHitDigi_factory(std::vector<std::string> default_input_tags, PhotoMultiplierHitDigiConfig cfg) :
            JChainFactoryT<edm4eic::MCRecoTrackerHitAssociation, PhotoMultiplierHitDigiConfig>(std::move(default_input_tags), cfg) {}

        /** One time initialization **/
        void Init() override;

        /** On run change preparations **/
        void ChangeRun(const std::shared_ptr<const JEvent> &event) override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

    private:

        std::string                      m_plugin_name;
        eicrecon::PhotoMultiplierHitDigi m_digi_algo;       /// Actual digitisation algorithm
    };

}
