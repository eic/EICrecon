#include "TracksToParticles.h"
#include <algorithm>

// Event Model related classes
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <edm4eic/TrackParametersCollection.h>
#include "JugTrack/IndexSourceLink.hpp"
#include "JugTrack/Track.hpp"

void eicrecon::Reco::TracksToParticles::init(std::shared_ptr<spdlog::logger> log)
{
  m_log = log;
}

std::unique_ptr<edm4eic::ReconstructedParticleCollection>
eicrecon::Reco::TracksToParticles::execute(const std::vector<const edm4eic::Track*>& tracks)
{
  auto rec_particles = std::make_unique<edm4eic::ReconstructedParticleCollection>();

  for(auto& track : tracks)
    {
      auto part = rec_particles->create();
      part.setMomentum(track->getMomentum());
      part.setCharge(track->getCharge());
    }

  return rec_particles;
}
