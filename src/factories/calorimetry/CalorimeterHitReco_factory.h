// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once

#include <algorithms/calorimetry/CalorimeterHitReco.h>
#include <services/geometry/dd4hep/DD4hep_service.h>
#include <extensions/jana/JOmniFactory.h>


namespace eicrecon {

class CalorimeterHitReco_factory : public JOmniFactory<CalorimeterHitReco_factory, CalorimeterHitRecoConfig> {

private:
    CalorimeterHitReco m_algo;

    PodioInput<edm4hep::RawCalorimeterHit> m_raw_hits_input {this};
    PodioOutput<edm4eic::CalorimeterHit> m_rec_hits_output {this};

    ParameterRef<unsigned int> m_capADC {this, "capacityADC", config().capADC};
    ParameterRef<double> m_dyRangeADC {this, "dynamicRangeADC", config().dyRangeADC};
    ParameterRef<unsigned int> m_pedMeanADC {this, "pedestalMean", config().pedMeanADC};
    ParameterRef<double> m_pedSigmaADC {this, "pedestalSigma", config().pedSigmaADC};
    ParameterRef<double> m_resolutionTDC {this, "resolutionTDC", config().resolutionTDC};
    ParameterRef<double> m_thresholdFactor {this, "thresholdFactor", config().thresholdFactor};
    ParameterRef<double> m_thresholdValue {this, "thresholdValue", config().thresholdValue};
    ParameterRef<double> m_samplingFraction {this, "samplingFraction", config().sampFrac};
    ParameterRef<std::string> m_readout {this, "readout", config().readout};
    ParameterRef<std::string> m_layerField {this, "layerField", config().layerField};
    ParameterRef<std::string> m_sectorField {this, "sectorField", config().sectorField};
    ParameterRef<std::string> m_localDetElement {this, "localDetElement", config().localDetElement};
    ParameterRef<std::vector<std::string>> m_localDetFields {this, "localDetFields", config().localDetFields};

    Service<DD4hep_service> m_geoSvc {this};


public:
    void Configure() {
        m_algo.applyConfig(config());
        m_algo.init(m_geoSvc().detector(), m_geoSvc().converter(), logger());
    }

    void ChangeRun(int64_t run_number) {
    }
    
    void Process(int64_t run_number, uint64_t event_number) {
        m_rec_hits_output() = m_algo.process(*m_raw_hits_input());
    }
};

} // eicrecon
