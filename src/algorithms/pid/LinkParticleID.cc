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

  return out_particles;
}
