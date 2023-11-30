// Copyright 2023, Wouter Deconinck
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

#include "algorithms/calorimetry/CalorimeterHitDigi.h"
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "extensions/jana/JOmniFactory.h"
#include "extensions/spdlog/SpdlogMixin.h"


namespace eicrecon {

class CalorimeterHitDigi_factory : public JOmniFactory<CalorimeterHitDigi_factory, CalorimeterHitDigiConfig> {

public:
    using AlgoT = eicrecon::CalorimeterHitDigi;
private:
    std::unique_ptr<AlgoT> m_algo;

    PodioInput<edm4hep::SimCalorimeterHit> m_hits_input {this};
    PodioOutput<edm4hep::RawCalorimeterHit> m_hits_output {this};

    ParameterRef<std::vector<double>> m_energyResolutions {this, "energyResolutions", config().eRes};
    ParameterRef<double> m_timeResolution {this, "timeResolution", config().tRes};
    ParameterRef<unsigned int> m_capADC {this, "capacityADC", config().capADC};
    ParameterRef<double> m_dyRangeADC {this, "dynamicRangeADC", config().dyRangeADC};
    ParameterRef<unsigned int> m_pedMeanADC {this, "pedestalMean", config().pedMeanADC};
    ParameterRef<double> m_pedSigmaADC {this, "pedestalSigma", config().pedSigmaADC};
    ParameterRef<double> m_resolutionTDC {this, "resolutionTDC", config().resolutionTDC};
    ParameterRef<double> m_corrMeanScale {this, "scaleResponse", config().corrMeanScale};
    ParameterRef<std::vector<std::string>> m_fields {this, "signalSumFields", config().fields};
    ParameterRef<std::string> m_readout {this, "readoutClass", config().readout};

    Service<DD4hep_service> m_geoSvc {this};

public:
    void Configure() {
        m_algo = std::make_unique<AlgoT>(GetPrefix());
        m_algo->applyConfig(config());
        m_algo->init(m_geoSvc().detector(), logger());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_nr, uint64_t event_nr) {
        m_algo->process({m_hits_input()}, {m_hits_output().get()});
    }
};

} // eicrecon
