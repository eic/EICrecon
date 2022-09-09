//
// Created by romanov on 8/7/22.
//

#ifndef EICRECON_BARRELTRACKERRAWHIT_H
#define EICRECON_BARRELTRACKERRAWHIT_H

#include <edm4eic/RawTrackerHit.h>

class BarrelTrackerRawHit: public edm4eic::RawTrackerHit {
public:
    BarrelTrackerRawHit(uint64_t cellID, int32_t charge, int32_t timeStamp):
            edm4eic::RawTrackerHit(cellID, charge, timeStamp) {}

};


#endif //EICRECON_BARRELTRACKERRAWHIT_H
