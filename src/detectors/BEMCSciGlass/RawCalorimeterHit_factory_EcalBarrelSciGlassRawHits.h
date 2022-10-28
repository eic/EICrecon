// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <global/calorimetry/CalorimeterHitDigi_algorithm_factory.h>

using namespace dd4hep;


class RawCalorimeterHit_factory_EcalBarrelSciGlassRawHits : public CalorimeterHitDigi_algorithm_factory {

public:

    //------------------------------------------
    // Constructor
    RawCalorimeterHit_factory_EcalBarrelSciGlassRawHits() {
        // Set default values for all config. parameters in CalorimeterHitDigi algorithm
        m_input_tag = "EcalBarrelSciGlassHits";
        u_eRes =  {};
        m_tRes = 0.0 * ns;
        m_capADC = 8096;
        m_dyRangeADC = 100 * MeV;
        m_pedMeanADC = 400;
        m_pedSigmaADC = 3.2;
        m_resolutionTDC = 10 * picosecond;
        m_corrMeanScale = 1.0;
        u_fields={};
        u_refs={};
        m_readout = "";

        // Output collection name:
        SetTag("EcalBarrelSciGlassRawHits");
    }
};

