// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//  Sections Copyright (C) 2022 Chao Peng, Wouter Deconinck, Sylvester Joosten
//  under SPDX-License-Identifier: LGPL-3.0-or-later

#include "BEMCRawCalorimeterHit_factory_utility.h"

#include <JANA/JEvent.h>
#include <edm4hep/SimCalorimeterHit.h>
#include <Evaluator/DD4hepUnits.h>
#include <fmt/format.h>
using namespace dd4hep;

//
// This algorithm converted from:
//
//  https://eicweb.phy.anl.gov/EIC/juggler/-/blob/master/JugDigi/src/components/CalorimeterHitDigi.cpp
//
// TODO:
// - Array type configuration parameters are not yet supported in JANA (needs to be added)
// - Random number service needs to bew resolved (on global scale)
// - It is possible standard running of this with Gaudi relied on a number of parameters
//   being set in the config. If that is the case, they should be moved into the default
//   values here. This needs to be confirmed.


//------------------------
// Constructor
//------------------------
BEMCRawCalorimeterHit_factory_utility::BEMCRawCalorimeterHit_factory_utility(){
    SetTag("utility");

    // This allows one to get the objects from this factory as edm4hep::RawCalorimeterHit.
    // This is useful for the EDM4hepWriter.
    EnableGetAs<edm4hep::RawCalorimeterHit>();
}

//------------------------
// Init
//------------------------
void BEMCRawCalorimeterHit_factory_utility::Init() {
    auto app = GetApplication();

    // I set the default values for the configuration parameters here, near the
    // calls to SetDefaultParameter. Mainly because that is what I'm used to.
    // The default values could also be set in the header file which would more
    // closely match the Gaudi style and may be a little cleaner.

    // TODO: There are 3 config values that are arrays that are not currently handled
    //Gaudi::Property<std::vector<double>> u_eRes{this, "energyResolutions", {}}; // a/sqrt(E/GeV) + b + c/(E/GeV)
    //Gaudi::Property<std::vector<std::string>> u_fields{this, "signalSumFields", {}};
    //Gaudi::Property<std::vector<int>>         u_refs{this, "fieldRefNumbers", {}};

    // Set default values for all config. parameters in CalorimeterHitDigi algorithm
    m_calhitdigi.m_input_tag     = "EMcalBarrelHits";
    m_calhitdigi.m_tRes          = 0.0 * ns;
    m_calhitdigi.m_tRes          = 0.0 * ns;
    m_calhitdigi.m_capADC        = 8096;
    m_calhitdigi.m_dyRangeADC    = 100 * MeV;
    m_calhitdigi.m_pedMeanADC    = 400;
    m_calhitdigi.m_pedSigmaADC   = 3.2;
    m_calhitdigi.m_resolutionTDC = 10 * picosecond;
    m_calhitdigi.m_corrMeanScale = 1.0;
    m_calhitdigi.m_geoSvcName = "GeoSvc";
    m_calhitdigi.m_readout    = "";
    m_calhitdigi.m_geoSvc = app->GetService<JDD4hep_service>(); // TODO: implement named geometry service?

    // This is one option for exposing the data members as JANA configuration parameters.
    // If using this, then one does not need all of the calls to app->SetDefaultParameter below
    m_calhitdigi.SetJANAConfigParameters(app, "BEMC");

//    // This is another option for exposing the data members as JANA configuration parameters.
//    app->SetDefaultParameter("BEMC:tag",             m_calhitdigi.m_input_tag, "tag/collection name for edm4hep::SimCalorimeterHit objects to use");
//    app->SetDefaultParameter("BEMC:timeResolution",  m_calhitdigi.m_tRes);
//    app->SetDefaultParameter("BEMC:capacityADC",     m_calhitdigi.m_capADC);
//    app->SetDefaultParameter("BEMC:dynamicRangeADC", m_calhitdigi.m_dyRangeADC);
//    app->SetDefaultParameter("BEMC:pedestalMean",    m_calhitdigi.m_pedMeanADC);
//    app->SetDefaultParameter("BEMC:pedestalSigma",   m_calhitdigi.m_pedSigmaADC);
//    app->SetDefaultParameter("BEMC:resolutionTDC",   m_calhitdigi.m_resolutionTDC);
//    app->SetDefaultParameter("BEMC:scaleResponse",   m_calhitdigi.m_corrMeanScale);
//    app->SetDefaultParameter("BEMC:geoServiceName",  m_calhitdigi.m_geoSvcName);
//    app->SetDefaultParameter("BEMC:readoutClass",    m_calhitdigi.m_readout);

    // Call Init for generic algorithm
    m_calhitdigi.AlgorithmInit();
}

//------------------------
// ChangeRun
//------------------------
void BEMCRawCalorimeterHit_factory_utility::ChangeRun(const std::shared_ptr<const JEvent> &event) {
    /// This is automatically run before Process, when a new run number is seen
    /// Usually we update our calibration constants by asking a JService
    /// to give us the latest data for this run number
    
    auto run_nr = event->GetRunNumber();
    // m_calibration = m_service->GetCalibrationsForRun(run_nr);

    // Call ChangeRun for generic algorithm
    m_calhitdigi.AlgorithmChangeRun();
}

//------------------------
// Process
//------------------------
void BEMCRawCalorimeterHit_factory_utility::Process(const std::shared_ptr<const JEvent> &event) {
    // Prefill inputs
    m_calhitdigi.simhits = event->Get<edm4hep::SimCalorimeterHit>( m_calhitdigi.m_input_tag );

    // Call Process for generic algorithm
    m_calhitdigi.AlgorithmProcess();

    // Convert generated objects into BEMCRawCalorimeterHit objects
    for( auto hit : m_calhitdigi.rawhits ) mData.push_back( new BEMCRawCalorimeterHit( hit ));

}

