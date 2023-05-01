// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023, Christopher Dilks

#pragma once

// JANA
#include <extensions/jana/JChainMultifactoryT.h>
#include <JANA/JEvent.h>

// data model
#include <edm4hep/SimTrackerHit.h>
#include <edm4eic/RawTrackerHit.h>
#include <edm4eic/MCRecoTrackerHitAssociation.h>

// algorithms
#include <algorithms/digi/PhotoMultiplierHitDigi.h>
#include <algorithms/digi/PhotoMultiplierHitDigiConfig.h>

// services
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <services/geometry/richgeo/RichGeo_service.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <extensions/string/StringHelpers.h>

namespace eicrecon {

    class PhotoMultiplierHitDigi;

    class PhotoMultiplierHitDigi_factory :
            public JChainMultifactoryT<PhotoMultiplierHitDigiConfig>,
            public SpdlogMixin<PhotoMultiplierHitDigi_factory> {

    public:

        explicit PhotoMultiplierHitDigi_factory(
            std::string tag,
            const std::vector<std::string>& input_tags,
            const std::vector<std::string>& output_tags,
            PhotoMultiplierHitDigiConfig cfg
            ):
          JChainMultifactoryT<PhotoMultiplierHitDigiConfig>(std::move(tag), input_tags, output_tags, cfg) {
            DeclarePodioOutput<edm4eic::RawTrackerHit>(GetOutputTags()[0]);
            DeclarePodioOutput<edm4eic::MCRecoTrackerHitAssociation>(GetOutputTags()[1]);
          }

        /** One time initialization **/
        void Init() override;

        /** On run change preparations **/
        void BeginRun(const std::shared_ptr<const JEvent> &event) override;

        /** Event by event processing **/
        void Process(const std::shared_ptr<const JEvent> &event) override;

    private:

        eicrecon::PhotoMultiplierHitDigi m_digi_algo;       /// Actual digitisation algorithm
        richgeo::ReadoutGeo              *m_readoutGeo = nullptr;
    };

}
