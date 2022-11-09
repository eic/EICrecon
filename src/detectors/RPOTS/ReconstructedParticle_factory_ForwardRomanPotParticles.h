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

    std::shared_ptr<spdlog::logger> m_logger;

    //------------------------------------------
    // Constructor
    ReconstructedParticle_factory_ForwardRomanPotParticles() {
        SetTag("ForwardRomanPotParticles");
        m_logger = japp->GetService<Log_service>()->logger(GetTag());
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

        app->SetDefaultParameter("RPOTS:ForwardRomanPotParticles:local_x_offset_station_1",local_x_offset_station_1);
        app->SetDefaultParameter("RPOTS:ForwardRomanPotParticles:local_x_offset_station_2",local_x_offset_station_2);
        app->SetDefaultParameter("RPOTS:ForwardRomanPotParticles:local_x_slope_offset",local_x_slope_offset);
        app->SetDefaultParameter("RPOTS:ForwardRomanPotParticles:local_y_slope_offset",local_y_slope_offset);
        app->SetDefaultParameter("RPOTS:ForwardRomanPotParticles:crossingAngle",crossingAngle);
        app->SetDefaultParameter("RPOTS:ForwardRomanPotParticles:nomMomentum",nomMomentum);
        app->SetDefaultParameter("RPOTS:ForwardRomanPotParticles:m_readout",m_readout);
        app->SetDefaultParameter("RPOTS:ForwardRomanPotParticles:m_layerField",m_layerField);
        app->SetDefaultParameter("RPOTS:ForwardRomanPotParticles:m_sectorField",m_sectorField);
        app->SetDefaultParameter("RPOTS:ForwardRomanPotParticles:m_localDetElement",m_localDetElement);
        app->SetDefaultParameter("RPOTS:ForwardRomanPotParticles:u_localDetFields",u_localDetFields);

        // Connect generic algorithm to dd4hep
        auto geomSrvc = app->GetService<JDD4hep_service>();
        detector = geomSrvc->detector();
        m_cellid_converter = geomSrvc->cellIDPositionConverter();

        // Call init for generic algorithm
        initialize(m_logger);
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
