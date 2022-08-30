// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_CKFTRACKINGCONFIG_H
#define EICRECON_CKFTRACKINGCONFIG_H

#include <vector>

namespace eicrecon {
    struct CKFTrackingConfig {
        std::vector<double> m_etaBins;  // {this, "etaBins", {}};
        std::vector<double> m_chi2CutOff; //{this, "chi2CutOff", {15.}};
        std::vector<size_t> m_numMeasurementsCutOff; //{this, "numMeasurementsCutOff", {10}};
    };
}



#endif //EICRECON_CKFTRACKINGCONFIG_H
