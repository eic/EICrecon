// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//


#ifndef _EcalBarrelRawCalorimeterHit_h_
#define _EcalBarrelRawCalorimeterHit_h_

#include <edm4hep/RawCalorimeterHit.h>

#include <JANA/JObject.h>

// TODO: Could this be done with just a typdef ? (probably not)

class EcalBarrelRawCalorimeterHit : public edm4hep::RawCalorimeterHit {

public:
    /// Make it convenient to construct one of these things
    // TODO: make this work with a variadic template so we don't have to modify this file if the data model changes
    EcalBarrelRawCalorimeterHit(std::uint64_t cellID, std::int32_t amplitude, std::int32_t timeStamp) : edm4hep::RawCalorimeterHit(cellID, amplitude,timeStamp) {};
};


#endif // _EcalBarrelRawCalorimeterHit_h_

