// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
#pragma once

#include <JANA/JEvent.h>
#include <JANA/JFactoryT.h>
#include <services/log/Log_service.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <algorithms/tracking/TrackerHitReconstruction.h>
#include <algorithms/tracking/TrackerHitReconstructionConfig.h>
#include <edm4hep/TrackerHit.h>
#include <edm4eic/RawTrackerHit.h>
#include <Evaluator/DD4hepUnits.h>
using namespace dd4hep;


class TrackerHit_factory_ForwardRomanPotRecHits : public JFactoryT<edm4eic::TrackerHit>, eicrecon::TrackerHitReconstruction {

public:

    std::shared_ptr<spdlog::logger> m_logger;

    //------------------------------------------
    // Constructor
    TrackerHit_factory_ForwardRomanPotRecHits() {
        SetTag("ForwardRomanPotRecHits");
        m_logger = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override {
        auto app = GetApplication();

        // Set default values for all config. parameters in CalorimeterHitDigi algorithm
        auto &config = getConfig();
        config.time_resolution = 0.0;
        app->SetDefaultParameter("RPOTS:ForwardRomanPotRawHits:time_resolution",config.time_resolution );

        // Call init for generic algorithm
        auto geoSvc = app->GetService<JDD4hep_service>();
        auto detector = geoSvc->detector();
        init(detector, m_logger);
    }

    //------------------------------------------
    // Process
    void Process(const std::shared_ptr<const JEvent> &event) override {
        // Get inputs
        auto rawhits = event->Get<edm4eic::RawTrackerHit>("ForwardRomanPotRawHits");

        // Call Process for generic algorithm
        std::vector<edm4eic::TrackerHit*> trackerhits;
        for( auto rawhit : rawhits) trackerhits.push_back(produce(rawhit));

        // Hand owner of algorithm objects over to JANA
        Set(trackerhits);
    }
};
