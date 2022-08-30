// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_BARRELTRACKERHITRECONSTRUCTION_H
#define EICRECON_BARRELTRACKERHITRECONSTRUCTION_H

#include <eicd/MutableTrackerHit.h>

class BarrelTrackerHitReconstruction: public eicd::TrackerHit{
public:
    BarrelTrackerHitReconstruction(std::uint64_t cellID, eicd::Vector3f position, eicd::CovDiag3f positionError, float time, float timeError, float edep, float edepError):
            MutableTrackerHit(cellID, position, positionError, time, timeError, edep, edepError)
    {
    }
};


#endif //EICRECON_BARRELTRACKERHITRECONSTRUCTION_H
