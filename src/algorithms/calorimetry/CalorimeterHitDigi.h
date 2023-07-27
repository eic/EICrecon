// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng, Wouter Deconinck, Sylvester Joosten, Barak Schmookler, David Lawrence

// A general digitization for CalorimeterHit from simulation
// 1. Smear energy deposit with a/sqrt(E/GeV) + b + c/E or a/sqrt(E/GeV) (relative value)
// 2. Digitize the energy with dynamic ADC range and add pedestal (mean +- sigma)
// 3. Time conversion with smearing resolution (absolute value)
// 4. Signal is summed if the SumFields are provided
//
// Author: Chao Peng
// Date: 06/02/2021


#pragma once

#include <memory>
#include <random>

#include "services/geometry/dd4hep/JDD4hep_service.h"

#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4hep/RawCalorimeterHitCollection.h>
#include <spdlog/spdlog.h>

class CalorimeterHitDigi {

    // Insert any member variables here

public:
    CalorimeterHitDigi() = default;
    void AlgorithmInit(std::shared_ptr<spdlog::logger>& logger);
    void AlgorithmChangeRun() ;
    std::unique_ptr<edm4hep::RawCalorimeterHitCollection> AlgorithmProcess(const edm4hep::SimCalorimeterHitCollection &simhits) ;

    //-------- Configuration Parameters ------------
    //instantiate new spdlog logger
    std::shared_ptr<spdlog::logger> m_log;

    // Name of input data type (collection)
    std::string              m_input_tag;

    // additional smearing resolutions
    std::vector<double>      u_eRes;
    double                   m_tRes;

    // single hit energy deposition threshold
    double                   m_threshold=1.0*dd4hep::keV; // {this, "threshold", 1. * keV};

    // digitization settings
    unsigned int             m_capADC;
    double                   m_capTime = 1000.;
    double                   m_dyRangeADC;
    unsigned int             m_pedMeanADC;
    double                   m_pedSigmaADC;
    double                   m_resolutionTDC;
    double                   m_corrMeanScale;

    // signal sums
    std::vector<std::string> u_fields;
    std::string              m_geoSvcName;
    std::string              m_readout;

    // This may be used to declare the data members as JANA configuration parameters.
    // This should compile OK even without JANA so long as you don't try using it.
    // To use it, do something like the following:
    //
    //    mycalohitdigi->SetJANAConfigParameters( japp, "BEMC");
    //
    // The above will register config. parameters like: "BEMC:tag".
    // The configuration parameter members of this class should be set to their
    // defaults *before* calling this.
    //-----------------------------------------------

    // unitless counterparts of inputs
    double           dyRangeADC{0}, stepTDC{0}, tRes{0};
    // variables for merging at digitization step
    bool             merge_hits = false;
    std::shared_ptr<JDD4hep_service> m_geoSvc;
    uint64_t         id_mask{0};

private:
    std::default_random_engine generator; // TODO: need something more appropriate here
    std::normal_distribution<double> m_normDist; // defaults to mean=0, sigma=1

    std::unique_ptr<edm4hep::RawCalorimeterHitCollection> single_hits_digi(const edm4hep::SimCalorimeterHitCollection &simhits);
    std::unique_ptr<edm4hep::RawCalorimeterHitCollection> signal_sum_digi(const edm4hep::SimCalorimeterHitCollection &simhits);
};
