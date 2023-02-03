// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <memory>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/TrackParametersCollection.h>

class ParticlesFromTrackFitResult {
public:
    ParticlesFromTrackFitResult(
            std::unique_ptr<edm4eic::ReconstructedParticleCollection> particles,
            std::unique_ptr<edm4eic::TrackParametersCollection> track_parameters):
            m_particles(std::move(particles)),
            m_track_parameters(std::move(track_parameters))
    {}

    [[nodiscard]] edm4eic::ReconstructedParticleCollection* particles() const {return m_particles.get();}
    [[nodiscard]] edm4eic::TrackParametersCollection* trackParameters() const {return m_track_parameters.get();}

private:
    std::unique_ptr<edm4eic::ReconstructedParticleCollection> m_particles;
    std::unique_ptr<edm4eic::TrackParametersCollection> m_track_parameters;
};


