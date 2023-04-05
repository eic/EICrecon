// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <vector>

namespace eicrecon {
    struct CKFTrackingConfig {
        std::vector<double> m_etaBins = {};  // {this, "etaBins", {}};
        std::vector<double> m_chi2CutOff = {15.}; //{this, "chi2CutOff", {15.}};
        std::vector<size_t> m_numMeasurementsCutOff = {10}; //{this, "numMeasurementsCutOff", {10}};
    };
}
