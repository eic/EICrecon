// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//  Sections Copyright (C) 2022 Chao Peng, Wouter Deconinck, Sylvester Joosten
//  under SPDX-License-Identifier: LGPL-3.0-or-later

#include "BEMCRawCalorimeterHit_factory.h"

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
BEMCRawCalorimeterHit_factory::BEMCRawCalorimeterHit_factory(){
    SetTag("");

    // This allows one to get the objects from this factory as edm4hep::RawCalorimeterHit.
    // This is useful for the EDM4hepWriter.
    EnableGetAs<edm4hep::RawCalorimeterHit>();
}

//------------------------
// Init
//------------------------
void BEMCRawCalorimeterHit_factory::Init() {
    auto app = GetApplication();

    // TODO: There are 3 config values that are arrays that are not currently handled
    //Gaudi::Property<std::vector<double>> u_eRes{this, "energyResolutions", {}}; // a/sqrt(E/GeV) + b + c/(E/GeV)
    //Gaudi::Property<std::vector<std::string>> u_fields{this, "signalSumFields", {}};
    //Gaudi::Property<std::vector<int>>         u_refs{this, "fieldRefNumbers", {}};

    // Set default values for all config. parameters in CalorimeterHitDigi algorithm
    m_input_tag     = "EcalBarrelHits";
    m_tRes          = 0.0 * ns;
    m_tRes          = 0.0 * ns;
    m_capADC        = 8096;
    m_dyRangeADC    = 100 * MeV;
    m_pedMeanADC    = 400;
    m_pedSigmaADC   = 3.2;
    m_resolutionTDC = 10 * picosecond;
    m_corrMeanScale = 1.0;
    m_geoSvcName = "GeoSvc";
    m_readout    = "";
    m_geoSvc = app->GetService<JDD4hep_service>(); // TODO: implement named geometry service?

    // This is one option for exposing the data members as JANA configuration parameters.
    // If using this, then one does not need all of the calls to app->SetDefaultParameter below
    SetJANAConfigParameters(app, "BEMC");

//    // This is another option for exposing the data members as JANA configuration parameters.
//    app->SetDefaultParameter("BEMC:tag",             m_input_tag, "tag/collection name for edm4hep::SimCalorimeterHit objects to use");
//    app->SetDefaultParameter("BEMC:timeResolution",  m_tRes);
//    app->SetDefaultParameter("BEMC:capacityADC",     m_capADC);
//    app->SetDefaultParameter("BEMC:dynamicRangeADC", m_dyRangeADC);
//    app->SetDefaultParameter("BEMC:pedestalMean",    m_pedMeanADC);
//    app->SetDefaultParameter("BEMC:pedestalSigma",   m_pedSigmaADC);
//    app->SetDefaultParameter("BEMC:resolutionTDC",   m_resolutionTDC);
//    app->SetDefaultParameter("BEMC:scaleResponse",   m_corrMeanScale);
//    app->SetDefaultParameter("BEMC:geoServiceName",  m_geoSvcName);
//    app->SetDefaultParameter("BEMC:readoutClass",    m_readout);

    // Call Init for generic algorithm
    AlgorithmInit();
}

//------------------------
// ChangeRun
//------------------------
void BEMCRawCalorimeterHit_factory::ChangeRun(const std::shared_ptr<const JEvent> &event) {
    /// This is automatically run before Process, when a new run number is seen
    /// Usually we update our calibration constants by asking a JService
    /// to give us the latest data for this run number
    
    auto run_nr = event->GetRunNumber();
    // m_calibration = m_service->GetCalibrationsForRun(run_nr);

    // Call ChangeRun for generic algorithm
    AlgorithmChangeRun();
}

//------------------------
// Process
//------------------------
void BEMCRawCalorimeterHit_factory::Process(const std::shared_ptr<const JEvent> &event) {

    // Prefill inputs
    simhits = event->Get<edm4hep::SimCalorimeterHit>( m_input_tag );

    // Call Process for generic algorithm
    AlgorithmProcess();

    // Convert generated objects into BEMCRawCalorimeterHit objects
    for( auto hit : rawhits ) mData.push_back( new BEMCRawCalorimeterHit( hit ));

}

