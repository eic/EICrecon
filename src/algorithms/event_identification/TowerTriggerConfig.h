
#pragma once
#include <cstdlib>
#include <edm4eic/unit_system.h>

struct TowerTriggerConfig {

    float timeframeWidth                 = 2000.0;
    float timesplitWidth                 = 20.0;
    float timeResolution_SiMaps          = 2000.0;
    float timeResolution_MPGD            = 30.0;
    float timeResolution_ACLGad          = 20.0;
    float timeResolution_EMCal           = 20.0;
    float timeResolution_HCal            = 100.0;
    double refInverseVelocity            = 0.0034;
    double backwardEtaMin                = -3.78;
    double backwardEtaMax                = -1.63;
    double barrelEtaMin                  = 1.80;
    double barrelEtaMax                  = 1.81;
    double forwardEtaMin                 = 1.77;
    double forwardEtaMax                 = 4.04;
    size_t ecalMultiplicityThreshold     = 10;
    size_t backwardTrackerMatchThreshold = 10;
    size_t barrelTrackerMatchThreshold   = 5;
    size_t forwardTrackerMatchThreshold  = 5;
    size_t trackerMultiplicityThreshold  = 1;
    double trigTimeWindowBef             = 10 * edm4eic::unit::ns;
    double trigTimeWindowAft             = 30 * edm4eic::unit::ns;
    double collisionTimeMarginBef        = 10 * edm4eic::unit::ns;
    double collisionTimeMarginAft        = 20 * edm4eic::unit::ns;
};
