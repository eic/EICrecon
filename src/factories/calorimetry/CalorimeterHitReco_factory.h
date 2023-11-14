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

    PodioInput<edm4eic::RawCalorimeterHit> m_raw_hits_input;
    PodioOutput<edm4eic::CalorimeterHit> m_rec_hits_output;

    ParameterRef m_capADC {this, "capacityADC", config().capADC};
    ParameterRef m_dyRangeADC {this, "dynamicRangeADC", config().dyRangeADC};
    ParameterRef m_pedMeanADC {this, "pedestalMean", config().pedMeanADC};
    ParameterRef m_pedSigmaADC {this, "pedestalSigma", config().pedSigmaADC};
    ParameterRef m_resolutionTDC {this, "resolutionTDC", config().resolutionTDC};
    ParameterRef m_thresholdFactor {this, "thresholdFactor", config().thresholdFactor};
    ParameterRef m_thresholdValue {this, "thresholdValue", config().thresholdValue};
    ParameterRef m_samplingFraction {this, "samplingFraction", config().sampFrac};
    ParameterRef m_readout {this, "readout", config().readout};
    ParameterRef m_layerField {this, "layerField", config().layerField};
    ParameterRef m_sectorField {this, "sectorField", config().sectorField};
    ParameterRef m_localDetElement {this, "localDetElement", config().localDetElement};
    ParameterRef m_localDefFields {this, "localDetFields", config().localDetFields};


public:
    void Configure() {
        m_algo.applyConfig(config());
        m_algo.init(m_geoSvc().detector(), logger());
    }
    
    void Process(int64_t run_number, uint64_t event_number) {
        m_rec_hits_output() = m_algo.process(m_raw_hits_input());
    }
};

} // eicrecon
