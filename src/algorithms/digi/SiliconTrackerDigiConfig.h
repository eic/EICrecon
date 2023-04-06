#pragma once

#include "DD4hep/DD4hepUnits.h"

namespace eicrecon {

    struct SiliconTrackerDigiConfig {
        double threshold  = 5 * dd4hep::keV;
        double timeResolution = 8;   /// TODO 8 of what units??? Same TODO in juggler. Probably [ns]
    };

} // eicrecon
