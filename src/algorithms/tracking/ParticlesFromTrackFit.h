// Original header license: SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Sylvester Joosten, Wouter Deconinck, Dmitry Romanov
//

#ifndef EICRECON_PARTICLESFROMTRACKFIT_H
#define EICRECON_PARTICLESFROMTRACKFIT_H


#include <spdlog/logger.h>
#include "JugTrack/Trajectories.hpp"
#include <algorithms/tracking/ParticlesFromTrackFitResult.h>


namespace Jug::Reco {

    /** Extract the particles form fit trajectories.
     *
     * \ingroup tracking
     */
    class ParticlesFromTrackFit {
    private:
        std::shared_ptr<spdlog::logger> m_log;


    public:
        void init(std::shared_ptr<spdlog::logger> log);

        ParticlesFromTrackFitResult *execute(const std::vector<const Jug::Trajectories *> &trajectories);

    };
} // namespace Jug::Reco

#endif //EICRECON_PARTICLESFROMTRACKFIT_H
