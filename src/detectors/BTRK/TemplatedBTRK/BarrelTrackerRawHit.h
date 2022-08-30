//
// Created by romanov on 8/7/22.
//

#ifndef EICRECON_BARRELTRACKERRAWHIT_H
#define EICRECON_BARRELTRACKERRAWHIT_H

#include <eicd/RawTrackerHit.h>

class BarrelTrackerRawHit: public eicd::RawTrackerHit {
public:
    BarrelTrackerRawHit(uint64_t cellID, int32_t charge, int32_t timeStamp):
            eicd::RawTrackerHit(cellID, charge, timeStamp) {}

};


#endif //EICRECON_BARRELTRACKERRAWHIT_H
