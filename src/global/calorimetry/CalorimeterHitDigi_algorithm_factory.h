// Copyright 2022, David Lawrence, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <random>

#include <JANA/JEvent.h>
#include <JANA/JFactoryT.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <algorithms/calorimetry/CalorimeterHitDigi.h>
#include <edm4hep/SimCalorimeterHit.h>
#include <edm4hep/RawCalorimeterHit.h>
#include <Evaluator/DD4hepUnits.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <extensions/string/StringHelpers.h>

using namespace dd4hep;


class CalorimeterHitDigi_algorithm_factory :
        public JFactoryT<edm4hep::RawCalorimeterHit>,
        public CalorimeterHitDigi,
        public eicrecon::SpdlogMixin<CalorimeterHitDigi_algorithm_factory>
{

public:

    //------------------------------------------
    // Init
    void Init() override {
        auto app = GetApplication();

        // Get log level from user parameter or default
        std::string parameter_prefix = eicrecon::str::ReplaceAll(GetPluginName(), ".so", "") + ":" + this->GetTag();
        m_geoSvc = app->GetService<JDD4hep_service>();

        // This is another option for exposing the data members as JANA configuration parameters.
        app->SetDefaultParameter(parameter_prefix + ":input_tag", m_input_tag, "Name of input collection to use");
        app->SetDefaultParameter(parameter_prefix + ":energyResolutions",u_eRes);
        app->SetDefaultParameter(parameter_prefix + ":timeResolution",   m_tRes);
        app->SetDefaultParameter(parameter_prefix + ":capacityADC",      m_capADC);
        app->SetDefaultParameter(parameter_prefix + ":dynamicRangeADC",  m_dyRangeADC);
        app->SetDefaultParameter(parameter_prefix + ":pedestalMean",     m_pedMeanADC);
        app->SetDefaultParameter(parameter_prefix + ":pedestalSigma",    m_pedSigmaADC);
        app->SetDefaultParameter(parameter_prefix + ":resolutionTDC",    m_resolutionTDC);
        app->SetDefaultParameter(parameter_prefix + ":scaleResponse",    m_corrMeanScale);
        app->SetDefaultParameter(parameter_prefix + ":signalSumFields",  u_fields);
        app->SetDefaultParameter(parameter_prefix + ":fieldRefNumbers",  u_refs);
        app->SetDefaultParameter(parameter_prefix + ":readoutClass",     m_readout);

        // Call Init for generic algorithm
        InitLogger(parameter_prefix);

        // Init underlying algorithm
        AlgorithmInit(logger());
    }

    //------------------------------------------
    // ChangeRun
    void ChangeRun(const std::shared_ptr<const JEvent> &event) override {
        AlgorithmChangeRun();
    }

    //------------------------------------------
    // Process
    void Process(const std::shared_ptr<const JEvent> &event) override {
        // Prefill inputs
        simhits = event->Get<edm4hep::SimCalorimeterHit>(m_input_tag);

        // Call Process for generic algorithm
        AlgorithmProcess();

        // Hand owner of algorithm objects over to JANA
        Set(rawhits);
        rawhits.clear(); // not really needed, but better to not leave dangling pointers around
    }
};

