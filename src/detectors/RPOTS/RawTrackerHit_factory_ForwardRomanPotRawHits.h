// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
#pragma once

#include <JANA/JEvent.h>
#include <JANA/JFactoryT.h>
#include <services/log/Log_service.h>
#include <algorithms/digi/SiliconTrackerDigi.h>
#include <algorithms/digi/SiliconTrackerDigiConfig.h>
#include <edm4hep/SimTrackerHit.h>
#include <edm4eic/RawTrackerHit.h>
#include <Evaluator/DD4hepUnits.h>
using namespace dd4hep;


class RawTrackerHit_factory_ForwardRomanPotRawHits : public JFactoryT<edm4eic::RawTrackerHit>, eicrecon::SiliconTrackerDigi {

public:

    //------------------------------------------
    // Constructor
    RawTrackerHit_factory_ForwardRomanPotRawHits() {
        SetTag("ForwardRomanPotRawHits");
    }

    //------------------------------------------
    // Init
    void Init() override {
        auto app = GetApplication();

        // Set default values for all config. parameters in CalorimeterHitDigi algorithm
        auto &config = getConfig();
        config.threshold = 0.0;
        config.timeResolution = 8.0; // units are ns

        app->SetDefaultParameter("RPOTS:ForwardRomanPotRawHits:threshold",     config.threshold );
        app->SetDefaultParameter("RPOTS:ForwardRomanPotRawHits:timeResolution",config.timeResolution );

        // Call init for generic algorithm
        auto logSrvc = app->GetService<Log_service>();
        auto logger = logSrvc->logger("RPOTS");
        init(logger);
    }

    //------------------------------------------
    // Process
    void Process(const std::shared_ptr<const JEvent> &event) override {
        // Get inputs
        auto sim_hits = event->Get<edm4hep::SimTrackerHit>("ForwardRomanPotHits");

        // Call Process for generic algorithm
        auto rawhits = produce( sim_hits );

        // Hand owner of algorithm objects over to JANA
        Set(rawhits);
    }
};
