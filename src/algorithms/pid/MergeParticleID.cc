// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "MergeParticleID.h"

// AlgorithmInit
//---------------------------------------------------------------------------
void eicrecon::MergeParticleID::AlgorithmInit(std::shared_ptr<spdlog::logger>& logger)
{
  m_log = logger;
  m_cfg.Print(m_log, spdlog::level::debug);
}


// AlgorithmChangeRun
//---------------------------------------------------------------------------
void eicrecon::MergeParticleID::AlgorithmChangeRun() {
}


// AlgorithmProcess
//---------------------------------------------------------------------------
std::vector<edm4eic::CherenkovParticleID*> eicrecon::MergeParticleID::AlgorithmProcess(
    std::vector<const edm4eic::CherenkovParticleID*>& in_pids
    )
{
  // logging
  m_log->trace("{:=^70}"," call MergeParticleID::AlgorithmProcess ");

  // start output collection
  std::vector<edm4eic::CherenkovParticleID*> out_pids;

  // loop over input collection, building `charged_particle_2_cherenkov_pids`, which maps a charged particle object ID
  // to the list of `CherenkovParticleID`s associated to it
  std::unordered_map<unsigned int, std::vector<const edm4eic::CherenkovParticleID*>> charged_particle_2_cherenkov_pids;
  for(const auto& in_pid : in_pids) {
    auto charged_particle_id = in_pid->getChargedParticle().id();
    auto pid_map_it = charged_particle_2_cherenkov_pids.find(charged_particle_id);
    if(pid_map_it != charged_particle_2_cherenkov_pids.end())
      pid_map_it->second.push_back(in_pid);
    else
      charged_particle_2_cherenkov_pids.insert({charged_particle_id,{in_pid}});
  }

  // loop over charged particles, combine weights from the associated Cherenkov PIDs,
  // and create output `CherenkovParticleID` objects
  for(auto [charged_particle_id, cherenkov_pids] : charged_particle_2_cherenkov_pids) {

    // logging
    m_log->trace("Charged Particle:");
    m_log->trace("  id = {}", charged_particle_id);
    m_log->trace("  PID Hypotheses:");

    // create output `CherenkovParticleID` object
    edm4eic::MutableCherenkovParticleID out_pid{
      0,   // radiator (not used)
      0.0, // npe
      0.0, // refractiveIndex (not used)
      0.0  // photonEnergy
    };

    // merge `CherenkovParticleID` objects from each radiator to one: `out_pid`
    for(int i=0; i<cherenkov_pids.size(); i++) {
      auto in_pid = cherenkov_pids[i];
      m_log->trace("    radiator {} hypothesis weights:", in_pid->getRadiator());
      out_pid.setNpe(out_pid.getNpe() + in_pid->getNpe());
      out_pid.setPhotonEnergy(in_pid->getNpe() * in_pid->getPhotonEnergy()); // (weighted average numerator summand)

      // merge PDG hypotheses, combining their weights and other members
      for(auto in_hyp : in_pid->getHypotheses()) {
        m_log->trace("      PDG = {:<6} weight = {:<10.8}", in_hyp.PDG, in_hyp.weight);
        if(i==0)
          out_pid.addToHypotheses(in_hyp);
        else {
          // find the hypothesis for this PDG
          for(auto out_hyp : out_pid.getHypotheses()) {
            if(out_hyp.PDG == in_hyp.PDG) {
              out_hyp.npe += in_hyp.npe;
              // combine hypotheses' weights
              switch(m_cfg.mergeMode) {
                case MergeParticleIDConfig::kAddWeights:
                  out_hyp.weight += in_hyp.weight;
                  break;
                case MergeParticleIDConfig::kMultiplyWeights:
                  out_hyp.weight *= in_hyp.weight;
                  break;
                default:
                  m_log->error("unknown MergeParticleIDConfig::mergeMode setting; weights not combined");
              }
            }
          }
        }
      }

      // finish computing averages
      if(out_pid.getNpe() > 0)
        out_pid.setPhotonEnergy(out_pid.getPhotonEnergy() / out_pid.getNpe());

      // add to `out_pids`
      m_log->trace("    combined hypothesis weights:");
      for(auto hyp : out_pid.getHypotheses())
        m_log->trace("      PDG = {:<6} weight = {:<10.8}", hyp.PDG, hyp.weight);
      out_pids.push_back(new edm4eic::CherenkovParticleID(out_pid)); // force immutable

    } // end `chernekov_pids` loop
  } // end charged_particle_2_cherenkov_pids loop

  return out_pids;
}
