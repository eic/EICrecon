#ifndef EICRECON_BARRELTRACKERSIMHIT_H
#define EICRECON_BARRELTRACKERSIMHIT_H

#include <edm4hep/SimTrackerHit.h>

class BarrelTrackerSimHit: public edm4hep::SimTrackerHit {
public:
    BarrelTrackerSimHit(std::uint64_t cellID, float EDep, float time, float pathLength, std::int32_t quality, edm4hep::Vector3d position, edm4hep::Vector3f momentum):
            edm4hep::SimTrackerHit(cellID, EDep, time, pathLength, quality, position, momentum)
    {}
};


#endif //EICRECON_BARRELTRACKERSIMHIT_H
