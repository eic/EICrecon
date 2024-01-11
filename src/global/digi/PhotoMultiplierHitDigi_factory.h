// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023, Christopher Dilks

#pragma once

#include <JANA/JEvent.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// algorithms
#include "algorithms/digi/PhotoMultiplierHitDigi.h"
#include "algorithms/digi/PhotoMultiplierHitDigiConfig.h"
// JANA
#include "extensions/jana/JOmniFactory.h"
// services
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "services/geometry/richgeo/RichGeo_service.h"
#include "services/geometry/richgeo/ReadoutGeo.h"

namespace eicrecon {

class PhotoMultiplierHitDigi_factory :
        public JOmniFactory<PhotoMultiplierHitDigi_factory, PhotoMultiplierHitDigiConfig> {

private:
    PhotoMultiplierHitDigi m_algo;

    PodioInput<edm4hep::SimTrackerHit> m_sim_hits_input {this};
    PodioOutput<edm4eic::RawTrackerHit> m_raw_hits_output {this};
    PodioOutput<edm4eic::MCRecoTrackerHitAssociation> m_raw_assocs_output {this};

    ParameterRef<unsigned long> m_seed {this, "seed", config().seed, "random number generator seed"};
    ParameterRef<double> m_hitTimeWindow {this, "hitTimeWindow", config().hitTimeWindow, ""};
    ParameterRef<double> m_timeResolution {this, "timeResolution", config().timeResolution, ""};
    ParameterRef<double> m_speMean {this, "speMean", config().speMean, ""};
    ParameterRef<double> m_speError {this, "speError", config().speError, ""};
    ParameterRef<double> m_pedMean {this, "pedMean", config().pedMean, ""};
    ParameterRef<double> m_pedError {this, "pedError", config().pedError, ""};
    ParameterRef<bool> m_enablePixelGaps {this, "enablePixelGaps", config().enablePixelGaps, "enable/disable removal of hits in gaps between pixels"};
    ParameterRef<double> m_safetyFactor {this, "safetyFactor", config().safetyFactor, "overall safety factor"};
    ParameterRef<bool> m_enableNoise {this, "enableNoise", config().enableNoise, ""};
    ParameterRef<double> m_noiseRate {this, "noiseRate", config().noiseRate, ""};
    ParameterRef<double> m_noiseTimeWindow {this, "noiseTimeWindow", config().noiseTimeWindow, ""};
    //ParameterRef<std::vector<std::pair<double, double>>> m_quantumEfficiency {this, "quantumEfficiency", config().quantumEfficiency, ""};

    Service<DD4hep_service> m_DD4hepSvc {this};
    Service<RichGeo_service> m_RichGeoSvc {this};

public:
    void Configure() {

        // Initialize richgeo ReadoutGeo and set random CellID visitor lambda (if a RICH)
        if (GetPluginName() == "DRICH" || GetPluginName() == "PFRICH") {
            m_RichGeoSvc().GetReadoutGeo(GetPluginName())->SetSeed(config().seed);
            m_algo.SetVisitRngCellIDs(
                [this] (std::function<void(PhotoMultiplierHitDigi::CellIDType)> lambda, float p) { m_RichGeoSvc().GetReadoutGeo(GetPluginName())->VisitAllRngPixels(lambda, p); }
                );
            m_algo.SetPixelGapMask(
                [this] (PhotoMultiplierHitDigi::CellIDType cellID, dd4hep::Position pos) { return m_RichGeoSvc().GetReadoutGeo(GetPluginName())->PixelGapMask(cellID, pos); }
                );
        }

        m_algo.applyConfig(config());
        m_algo.init(m_DD4hepSvc().detector(), m_DD4hepSvc().converter(), logger());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
        std::tie(m_raw_hits_output(), m_raw_assocs_output()) = m_algo.process(m_sim_hits_input());
    }
};

} // eicrecon
