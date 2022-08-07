//
// Created by romanov on 8/7/22.
//

#ifndef EICRECON_BARRELTRACKERRAWHIT_H
#define EICRECON_BARRELTRACKERRAWHIT_H

#include <algorithms/digi/RawTrackerHit.h>

class BarrelTrackerRawHit: public eicrecon::RawTrackerHit {
public:
    BarrelTrackerRawHit(uint64_t cellID, int32_t charge, int32_t timeStamp):
        eicrecon::RawTrackerHit(cellID, charge, timeStamp) {}

};


#endif //EICRECON_BARRELTRACKERRAWHIT_H
