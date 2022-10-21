// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
// This class wraps the generic FarForwardParticles algorithm for use in JANA2.

#pragma once

#include <JANA/JEvent.h>
#include <JANA/JFactoryT.h>
#include <services/log/Log_service.h>
#include <algorithms/romanpots/FarForwardParticles.h>


class ReconstructedParticle_factory_ForwardRomanPotParticles : public JFactoryT<edm4eic::ReconstructedParticle>, FarForwardParticles {

public:

    //------------------------------------------
    // Constructor
    ReconstructedParticle_factory_ForwardRomanPotParticles() {
        SetTag("ForwardRomanPotParticles");
    }

    //------------------------------------------
    // Init
    void Init() override {
        auto app = GetApplication();

        // Set default values for all config. parameters in CalorimeterHitDigi algorithm
        local_x_offset_station_1 = -833.3878326;
        local_x_offset_station_2 = -924.342804;
        local_x_slope_offset = -0.00622147;
        local_y_slope_offset = -0.0451035;
        crossingAngle = -0.025;
        nomMomentum = 275.0;

        m_readout="";
        m_layerField="";
        m_sectorField="";

        m_localDetElement="";
        u_localDetFields={};

        app->SetDefaultParameter("RPOTS:ForwardRomanPotRawHits:local_x_offset_station_1",local_x_offset_station_1);
        app->SetDefaultParameter("RPOTS:ForwardRomanPotRawHits:local_x_offset_station_2",local_x_offset_station_2);
        app->SetDefaultParameter("RPOTS:ForwardRomanPotRawHits:local_x_slope_offset",local_x_slope_offset);
        app->SetDefaultParameter("RPOTS:ForwardRomanPotRawHits:local_y_slope_offset",local_y_slope_offset);
        app->SetDefaultParameter("RPOTS:ForwardRomanPotRawHits:crossingAngle",crossingAngle);
        app->SetDefaultParameter("RPOTS:ForwardRomanPotRawHits:nomMomentum",nomMomentum);
        app->SetDefaultParameter("RPOTS:ForwardRomanPotRawHits:m_readout",m_readout);
        app->SetDefaultParameter("RPOTS:ForwardRomanPotRawHits:m_layerField",m_layerField);
        app->SetDefaultParameter("RPOTS:ForwardRomanPotRawHits:m_sectorField",m_sectorField);
        app->SetDefaultParameter("RPOTS:ForwardRomanPotRawHits:m_localDetElement",m_localDetElement);
        app->SetDefaultParameter("RPOTS:ForwardRomanPotRawHits:u_localDetFields",u_localDetFields);

        // Connect generic algorithm to dd4hep
        auto geomSrvc = app->GetService<JDD4hep_service>();
        detector = geomSrvc->detector();
        m_cellid_converter = geomSrvc->cellIDPositionConverter();

        // Call init for generic algorithm
        auto logSrvc = app->GetService<Log_service>();
        auto logger = logSrvc->logger("RPOTS");
        initialize(logger);
    }

    //------------------------------------------
    // Process
    void Process(const std::shared_ptr<const JEvent> &event) override {
        // Get inputs
        m_inputHits = event->Get<edm4eic::TrackerHit>("ForwardRomanPotRecHits");

        // Call Process for generic algorithm
        execute();

        // Hand ownership of algorithm objects over to JANA
        Set(m_outputParticles);
        m_outputParticles.clear();
    }
};
