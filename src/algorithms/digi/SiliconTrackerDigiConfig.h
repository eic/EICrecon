#pragma once

namespace eicrecon {

    struct SiliconTrackerDigiConfig {
        double threshold  = 0;
        double timeResolution = 8;   /// TODO 8 of what units??? Same TODO in juggler. Probably [ns]
    };

} // eicrecon
