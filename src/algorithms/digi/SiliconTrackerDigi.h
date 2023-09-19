// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck, Sylvester Joosten, Dmitry Romanov

#pragma once

#include <TRandomGen.h>
#include <functional>
#include <memory>

#include "SiliconTrackerDigiConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace edm4eic { class RawTrackerHitCollection; }
namespace edm4hep { class SimTrackerHitCollection; }
namespace spdlog { class logger; }

namespace eicrecon {

    /** digitization algorithm for a silicon trackers **/
    class SiliconTrackerDigi : public WithPodConfig<SiliconTrackerDigiConfig>  {

    public:
        void init(std::shared_ptr<spdlog::logger>& logger);
        std::unique_ptr<edm4eic::RawTrackerHitCollection> process(const edm4hep::SimTrackerHitCollection& sim_hits);

    private:
        /** algorithm logger */
        std::shared_ptr<spdlog::logger> m_log;

        /** Random number generation*/
        TRandomMixMax m_random;
        std::function<double()> m_gauss;

        // FIXME replace with standard random engine
        //std::default_random_engine generator; // TODO: need something more appropriate here
        //std::normal_distribution<double> m_normDist; // defaults to mean=0, sigma=1

    };

} // eicrecon
