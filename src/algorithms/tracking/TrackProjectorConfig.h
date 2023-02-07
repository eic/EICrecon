// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <string>

namespace eicrecon {

    struct TrackProjectorConfig {
        unsigned int m_firstInVolumeID;
        std::string m_firstInVolumeName;
        float m_firstSmallerThanZ;
        float m_firstGreaterThanZ;
        float m_firstGreaterThanR;
    };

} // eicrecon

