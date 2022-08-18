#ifndef EICRECON_SILICONTRACKERDIGICONFIG_H
#define EICRECON_SILICONTRACKERDIGICONFIG_H

namespace eicrecon {

    struct SiliconTrackerDigiConfig {
        double threshold  = 0;
        double timeResolution = 8;   /// TODO 8 of what units??? Same TODO in juggler. Probably [ns]
    };

} // eicrecon

#endif //EICRECON_SILICONTRACKERDIGICONFIG_H
