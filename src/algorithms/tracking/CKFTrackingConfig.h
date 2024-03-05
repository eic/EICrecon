// SPDX-License-Identifier: JSA
// Copyright (C) 2022, Dmitry Romanov

#pragma once

#include <vector>

namespace eicrecon {
    struct CKFTrackingConfig {
        std::vector<double> m_etaBins = {};  // {this, "etaBins", {}};
        std::vector<double> m_chi2CutOff = {15.}; //{this, "chi2CutOff", {15.}};
        std::vector<size_t> m_numMeasurementsCutOff = {10}; //{this, "numMeasurementsCutOff", {10}};
    };
}
