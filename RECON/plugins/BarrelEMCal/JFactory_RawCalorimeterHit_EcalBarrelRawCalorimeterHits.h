// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef _JFactory_RawCalorimeterHit_EcalBarrelRawCalorimeterHits_h_
#define _JFactory_RawCalorimeterHit_EcalBarrelRawCalorimeterHits_h_

#include <JANA/JFactoryT.h>

#include <edm4hep/RawCalorimeterHit.h>

class JFactory_RawCalorimeterHit_EcalBarrelRawCalorimeterHits : public JFactoryT<edm4hep::RawCalorimeterHit> {

    // Insert any member variables here

public:
    JFactory_RawCalorimeterHit_EcalBarrelRawCalorimeterHits();
    void Init() override;
    void ChangeRun(const std::shared_ptr<const JEvent> &event) override;
    void Process(const std::shared_ptr<const JEvent> &event) override;

    // -- Here would be identical to what is in JFactory_EcalBarrelRawCalorimeterHit.h ---

private:
    void single_hits_digi( const std::shared_ptr<const JEvent> &event );
    void signal_sum_digi( const std::shared_ptr<const JEvent> &event );

};

#endif // _JFactory_RawCalorimeterHit_EcalBarrelRawCalorimeterHits_h_
