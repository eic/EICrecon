// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_TRACKSOURCELINKERRESULT_H
#define EICRECON_TRACKSOURCELINKERRESULT_H

#include <algorithms/tracking/JugTrack/IndexSourceLink.hpp>
#include <algorithms/tracking/JugTrack/Measurement.hpp>

namespace eicrecon {
    struct TrackSourceLinkerResult {
        Jug::IndexSourceLinkContainer  sourceLinks;
        Jug::MeasurementContainer measurements;
    };
}




#endif //EICRECON_TRACKSOURCELINKERRESULT_H
