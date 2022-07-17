
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

};

#endif // _JFactory_RawCalorimeterHit_EcalBarrelRawCalorimeterHits_h_
