// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include "algorithms/tracking/JugTrack/GeometryContainers.hpp"
#include "algorithms/tracking/JugTrack/IndexSourceLink.hpp"
#include "algorithms/tracking/JugTrack/Measurement.hpp"

namespace eicrecon {
    struct TrackerSourceLinkerResult {
        std::shared_ptr<eicrecon::MeasurementContainer> measurements;
        std::vector<std::shared_ptr<eicrecon::IndexSourceLink>> sourceLinks;
    };
}
