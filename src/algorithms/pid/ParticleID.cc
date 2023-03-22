// Copyright 2023, Christopher Dilks, adapted from Alexander Kiselev's Juggler implementation `IRTAlgorithm`
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "ParticleID.h"

// AlgorithmInit
//---------------------------------------------------------------------------
void eicrecon::ParticleID::AlgorithmInit(std::shared_ptr<spdlog::logger>& logger)
{
  m_log = logger;
  m_cfg.Print(m_log, spdlog::level::debug);
}


// AlgorithmChangeRun
//---------------------------------------------------------------------------
void eicrecon::ParticleID::AlgorithmChangeRun() {
}


// AlgorithmProcess
//---------------------------------------------------------------------------
std::vector<edm4hep::ParticleID*> eicrecon::ParticleID::AlgorithmProcess(
    std::vector<const edm4eic::CherenkovParticleID*>& in_pids
    )
{
  // logging
  m_log->trace("{:=^70}"," call ParticleID::AlgorithmProcess ");

  // start output collection
  std::vector<edm4hep::ParticleID*> out_pids;

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
  // and create output `ParticleID` objects
  for(auto [charged_particle_id, cherenkov_pids] : charged_particle_2_cherenkov_pids) {

    // logging
    m_log->trace("Charged Particle:");
    m_log->trace("  id = {}", charged_particle_id);
    m_log->trace("  PID Hypotheses:");

    // combine weights for each PDG hypothesis
    std::unordered_map<
      decltype(edm4eic::CherenkovParticleIDHypothesis::PDG),
      decltype(edm4eic::CherenkovParticleIDHypothesis::weight)> pdg_2_combined_weight;
    for(int i=0; i<cherenkov_pids.size(); i++) {
      m_log->trace("    radiator {} hypothesis weights:", cherenkov_pids[i]->getRadiator());
      for(auto hyp : cherenkov_pids[i]->getHypotheses()) {
        m_log->trace("      PDG = {:<6} weight = {:<10.8}", hyp.PDG, hyp.weight);
        if(i==0)
          pdg_2_combined_weight.insert({ hyp.PDG, hyp.weight });
        else
          pdg_2_combined_weight.at(hyp.PDG) += hyp.weight; // FIXME: is this math correct????????????????????????????????
      }
    }
    m_log->trace("    combined hypothesis weights:");
    for(auto [pdg,weight] : pdg_2_combined_weight)
      m_log->trace("      PDG = {:<6} weight = {:<10.8}", pdg, weight);

    // create output `ParticleID` object
    if(m_cfg.highestWeightOnly) { // choose the highest weight only
      edm4hep::MutableParticleID out_pid;
      decltype(edm4eic::CherenkovParticleIDHypothesis::weight) max_weight = 0;
      decltype(edm4eic::CherenkovParticleIDHypothesis::PDG)    best_pdg   = 999999; // "unknown default" according to EDMhep
      for(auto [pdg,weight] : pdg_2_combined_weight) {
        if(weight > max_weight) {
          max_weight = weight;
          best_pdg   = pdg;
        }
      }
      out_pid.setType(          static_cast<decltype(edm4hep::ParticleIDData::type)>          (0)          ); // FIXME: unused
      out_pid.setAlgorithmType( static_cast<decltype(edm4hep::ParticleIDData::algorithmType)> (0)          ); // FIXME: unused
      out_pid.setPDG(           static_cast<decltype(edm4hep::ParticleIDData::PDG)>           (best_pdg)   );
      out_pid.setLikelihood(    static_cast<decltype(edm4hep::ParticleIDData::likelihood)>    (max_weight) );
      m_log->trace("    ParticleID object:");
      m_log->trace("      PDG = {:<6} weight = {:<10.8}", out_pid.getPDG(), out_pid.getLikelihood());
      out_pids.push_back(new edm4hep::ParticleID(out_pid)); // force immutable
    }
    else { // or take all of them
      m_log->error("highestWeightOnly==false not yet implemented"); // FIXME
    }
  }

  return out_pids;
}
