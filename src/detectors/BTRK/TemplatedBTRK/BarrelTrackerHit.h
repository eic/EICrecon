// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_BARRELTRACKERHITRECONSTRUCTION_H
#define EICRECON_BARRELTRACKERHITRECONSTRUCTION_H

#include <edm4eic/MutableTrackerHit.h>

class BarrelTrackerHitReconstruction: public edm4eic::TrackerHit{
public:
    BarrelTrackerHitReconstruction(std::uint64_t cellID, edm4eic::Vector3f position, edm4eic::CovDiag3f positionError, float time, float timeError, float edep, float edepError):
            MutableTrackerHit(cellID, position, positionError, time, timeError, edep, edepError)
    {
    }
};


#endif //EICRECON_BARRELTRACKERHITRECONSTRUCTION_H
