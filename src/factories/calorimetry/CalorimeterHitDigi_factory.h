// Copyright 2023, Wouter Deconinck
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include "algorithms/calorimetry/CalorimeterHitDigi.h"
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "extensions/jana/JOmniFactory.h"
#include "extensions/spdlog/SpdlogMixin.h"


namespace eicrecon {

class CalorimeterHitDigi_factory : public JOmniFactory<CalorimeterHitDigiConfig, CalorimeterHitDigiConfig> {

private:
    CalorimeterHitDigi m_algo;

    PodioInput<edm4hep::SimCalorimeterHit> m_hits_input;
    PodioOutput<edm4hep::RawCalorimeterHit> m_hits_output;

    ParameterRef<std::vector<double>> m_energyResolutions {this, "energyResolutions", config().eRes};
    ParameterRef<double> m_timeResolution {this, "timeResolution",   cfg.tRes);
    ParameterRef<unsigned int> m_capADC {this, "capacityADC", cfg.capADC);
    ParameterRef<double> m_dyRangeADC {this, "dynamicRangeADC", cfg.dyRangeADC);
    ParameterRef<unsigned int> m_pedMeanADC {this, "pedestalMean", cfg.pedMeanADC);
    ParameterRef<double> m_pedSigmaADC {this, "pedestalSigma", cfg.pedSigmaADC);
    ParameterRef<double> m_resolutionTDC {this, "resolutionTDC", cfg.resolutionTDC);
    ParameterRef<double> m_corrMeanScale {this, "scaleResponse", cfg.corrMeanScale);
    ParameterRef<std::vector<std::string>> m_fields {this, "signalSumFields", cfg.fields);
    ParameterRef<std::string> m_readout {this, "readoutClass", cfg.readout);

    Service<DD4hep_service> m_geoSvc {this};

public:
    void Configure() {
        m_algo.applyConfig(config());
        m_algo.init(m_geoSvc().detector(), logger());
    }

    void Process(int64_t run_nr, uint64_t event_nr) override {
        m_hits_output() = m_algo.process(m_hits_input());
    }
};

} // eicrecon
