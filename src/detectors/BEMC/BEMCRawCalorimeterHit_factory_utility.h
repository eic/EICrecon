// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef _JFactory_BEMCRawCalorimeterHit_utility_h_
#define _JFactory_BEMCRawCalorimeterHit_utility_h_

#include <random>

#include <JANA/JFactoryT.h>
#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <algorithms/caolrimetry/CalorimeterHitDigi.h>

#include "BEMCRawCalorimeterHit.h"

class JFactory_BEMCRawCalorimeterHit_utility : public JFactoryT<BEMCRawCalorimeterHit> {

    // Insert any member variables here

public:
    JFactory_BEMCRawCalorimeterHit_utility();
    void Init() override;
    void ChangeRun(const std::shared_ptr<const JEvent> &event) override;
    void Process(const std::shared_ptr<const JEvent> &event) override;


protected:
    CalorimeterHitDigi m_calhitdigi;
};

#endif // _JFactory_BEMCRawCalorimeterHit_utility_h_
