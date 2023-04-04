// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "LinkParticleID.h"

// AlgorithmInit
//---------------------------------------------------------------------------
void eicrecon::LinkParticleID::AlgorithmInit(std::shared_ptr<spdlog::logger>& logger)
{
  m_log = logger;
  m_cfg.Print(m_log, spdlog::level::debug);
}


// AlgorithmChangeRun
//---------------------------------------------------------------------------
void eicrecon::LinkParticleID::AlgorithmChangeRun() {
}


// AlgorithmProcess
//---------------------------------------------------------------------------
std::vector<edm4eic::ReconstructedParticle*> eicrecon::LinkParticleID::AlgorithmProcess(
    std::vector<const edm4eic::ReconstructedParticle*>& in_particles,
    std::vector<const edm4eic::CherenkovParticleID*>& in_pids
    )
{
  // logging
  m_log->trace("{:=^70}"," call LinkParticleID::AlgorithmProcess ");

  // start output collection
  std::vector<edm4eic::ReconstructedParticle*> out_particles;

  // loop over input reconstructed particles
  for(auto& in_particle : in_particles) {
    auto in_particle_p   = in_particle->getMomentum();
    auto in_particle_eta = edm4hep::utils::eta(in_particle_p);
    auto in_particle_phi = edm4hep::utils::angleAzimuthal(in_particle_p);
    m_log->trace("Input particle: (eta,phi) = ( {:>10.8}, {:>10.8} deg )", in_particle_eta, in_particle_phi / dd4hep::degree);
  }

  return out_particles;
}
