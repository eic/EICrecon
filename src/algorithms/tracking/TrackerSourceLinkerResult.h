// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include "ActsExamples/EventData/GeometryContainers.hpp"
#include "ActsExamples/EventData/IndexSourceLink.hpp"
#include "ActsExamples/EventData/Measurement.hpp"

namespace eicrecon {
    struct TrackerSourceLinkerResult {
        std::shared_ptr<eicrecon::MeasurementContainer> measurements;
        std::vector<std::shared_ptr<eicrecon::IndexSourceLink>> sourceLinks;
    };
}
