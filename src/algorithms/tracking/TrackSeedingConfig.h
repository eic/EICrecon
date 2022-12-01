// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_TRACK_SEEDING_CONFIG_H
#define EICRECON_TRACK_SEEDING_CONFIG_H

#include <vector>

namespace eicrecon {
    struct TrackSeedingConfig {
        std::vector<double> m_etaBins = {};  // {this, "etaBins", {}};
        std::vector<double> m_chi2CutOff = {50.}; //{this, "chi2CutOff", {15.}};
        std::vector<size_t> m_numMeasurementsCutOff = {10}; //{this, "numMeasurementsCutOff", {10}};
    };
}



#endif //EICRECON_CKFTRACKINGCONFIG_H
