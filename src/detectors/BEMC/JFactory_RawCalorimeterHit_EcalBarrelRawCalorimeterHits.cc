// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "JFactory_RawCalorimeterHit_EcalBarrelRawCalorimeterHits.h"

#include <JANA/JEvent.h>

//
// This algorithm converted from:
//
//  https://eicweb.phy.anl.gov/EIC/juggler/-/blob/master/JugDigi/src/components/CalorimeterHitDigi.cpp
//
// This is an alternative form for implementing this in JANA. I have left out most of
// the actual algorithm parts just to illustrate the differences. THose parts could be
// simply cut and pasted from JFactory_EcalBarrelRawCalorimeterHit.cc.

//------------------------
// Constructor
//------------------------
JFactory_RawCalorimeterHit_EcalBarrelRawCalorimeterHits::JFactory_RawCalorimeterHit_EcalBarrelRawCalorimeterHits(){
    SetTag("EcalBarrelRawCalorimeterHits");
}

//------------------------
// Init
//------------------------
void JFactory_RawCalorimeterHit_EcalBarrelRawCalorimeterHits::Init() {

    //---- This would be identical to JFactory_EcalBarrelRawCalorimeterHit::Init() ----

}

//------------------------
// ChangeRun
//------------------------
void JFactory_RawCalorimeterHit_EcalBarrelRawCalorimeterHits::ChangeRun(const std::shared_ptr<const JEvent> &event) {

    //---- This would be identical to JFactory_EcalBarrelRawCalorimeterHit::ChangeRun() ----

}

//------------------------
// Process
//------------------------
void JFactory_RawCalorimeterHit_EcalBarrelRawCalorimeterHits::Process(const std::shared_ptr<const JEvent> &event) {

    //---- This would be identical to JFactory_EcalBarrelRawCalorimeterHit::Process() ----

}

//------------------------
// single_hits_digi
//------------------------
void JFactory_RawCalorimeterHit_EcalBarrelRawCalorimeterHits::single_hits_digi( const std::shared_ptr<const JEvent> &event ) {

    //---- This would be identical to JFactory_EcalBarrelRawCalorimeterHit::single_hits_digi()
    //---- except that line:
    //----
    //----    std::vector<EcalBarrelRawCalorimeterHit*> rawhits;
    //----
    //---- would be changed to:

    std::vector<edm4hep::RawCalorimeterHit*> rawhits;

}

//------------------------
// signal_sum_digi
//------------------------
void JFactory_RawCalorimeterHit_EcalBarrelRawCalorimeterHits::signal_sum_digi( const std::shared_ptr<const JEvent> &event ) {

    //---- This would be identical to JFactory_EcalBarrelRawCalorimeterHit::signal_sum_digi()
    //---- except that line:
    //----
    //----    std::vector<EcalBarrelRawCalorimeterHit*> rawhits;
    //----
    //---- would be changed to:

    std::vector<edm4hep::RawCalorimeterHit*> rawhits;

}