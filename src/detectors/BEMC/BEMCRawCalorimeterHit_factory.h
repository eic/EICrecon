// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef _JFactory_BEMCRawCalorimeterHit_h_
#define _JFactory_BEMCRawCalorimeterHit_h_

#include <random>

#include <JANA/JFactoryT.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <algorithms/calorimetry/CalorimeterHitDigi.h>

#include "BEMCRawCalorimeterHit.h"

class BEMCRawCalorimeterHit_factory : public JFactoryT<BEMCRawCalorimeterHit>, CalorimeterHitDigi {

    // Insert any member variables here

public:
    BEMCRawCalorimeterHit_factory();
    void Init() override;
    void ChangeRun(const std::shared_ptr<const JEvent> &event) override;
    void Process(const std::shared_ptr<const JEvent> &event) override;


private:

};

#endif // _JFactory_BEMCRawCalorimeterHit_h_
