// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_TRACKPROJECTORCONFIG_H
#define EICRECON_TRACKPROJECTORCONFIG_H

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

#endif //EICRECON_TRACKPROJECTORCONFIG_H
