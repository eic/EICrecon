// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//


#ifndef _BEMCRawCalorimeterHit_h_
#define _BEMCRawCalorimeterHit_h_

#include <edm4hep/RawCalorimeterHit.h>


class BEMCRawCalorimeterHit : public edm4hep::RawCalorimeterHit {

public:
    /// Make it convenient to construct one of these things
    // TODO: make this work with a variadic template so we don't have to modify this file if the data model changes
    BEMCRawCalorimeterHit(std::uint64_t cellID, std::int32_t amplitude, std::int32_t timeStamp) : edm4hep::RawCalorimeterHit(cellID, amplitude,timeStamp) {};
    BEMCRawCalorimeterHit(const edm4hep::RawCalorimeterHit* obj): edm4hep::RawCalorimeterHit(*obj) {};
};


#endif // _BEMCRawCalorimeterHit_h_

