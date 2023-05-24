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
std::unique_ptr<edm4eic::CherenkovParticleIDCollection> eicrecon::MergeParticleID::AlgorithmProcess(
    std::vector<const edm4eic::CherenkovParticleIDCollection*> in_pid_collections_list
    )
{
  // logging
  m_log->trace("{:=^70}"," call MergeParticleID::AlgorithmProcess ");

  // start output collection
  auto out_pids = std::make_unique<edm4eic::CherenkovParticleIDCollection>();

  /* match input `CherenkovParticleIDCollection` elements from each list of
   * collections in `in_pid_collections_list`
   * - the matching is done by the ID of the charged particle object
   * - each unique charged particle object ID should correspond to a unique
   *   charged particle object, so any "merging" of the charged particle
   *   objects needs to be done upstream of this algorithm
   *
   * PROCEDURE:
   * - build data structure `particle_pids`, which maps a charged particle object
   *   ID to a list of index pairs referring to the `CherenkovParticleID`s
   *   associated to it
   *   - the 1st index of the pair is that of the input std::vector `in_pid_collections_list`
   *   - the 2nd index is that of the corresponding CherenkovParticleIDCollection
   *
   * EXAMPLE for merging dRICH aerogel and gas PID objects:
   *
   * - INPUT std::vector: `in_pid_collections_list`
   *   0. CherenkovParticleIDCollection: aerogel pids
   *      0. aerogel PID for charged particle A
   *      1. aerogel PID for charged particle B
   *   1. CherenkovParticleIDCollection: gas pids
   *      0. gas PID for charged particle A
   *      1. gas PID for charged particle B
   *      2. gas PID for charged particle C  // outside aerogel acceptance, but within gas acceptance
   *
   * - OUTPUT std::unordered_map: `particle_pids` (integer => std::vector<pair of integers>)
   *   - ID of charged particle A => { (0, 0), (1, 0) }
   *   - ID of charged particle B => { (0, 1), (1, 1) }
   *   - ID of charged particle C => { (1, 2) }
   */

  // fill `particle_pids`
  // -------------------------------------------------------------------------------
  std::unordered_map< unsigned int, std::vector<std::pair<size_t,size_t>> > particle_pids;
  m_log->trace("{:-<70}","Build `particle_pids` indexing data structure ");

  // loop over list of PID collections
  for(size_t idx_coll = 0; idx_coll < in_pid_collections_list.size(); idx_coll++) {
    const auto& in_pid_collection = in_pid_collections_list.at(idx_coll);
    m_log->trace("idx_col={}", idx_coll);

    // loop over this PID collection
    for(size_t idx_pid = 0; idx_pid < in_pid_collection->size(); idx_pid++) {

      // make the index pair
      const auto& in_pid = in_pid_collection->at(idx_pid);
      auto& charged_particle_track_segment = in_pid.getChargedParticle();
      if(!charged_particle_track_segment.isAvailable()) {
        m_log->error("PID object found with no charged particle");
        continue;
      }
      auto& charged_particle_track = charged_particle_track_segment.getTrack();
      if(!charged_particle_track.isAvailable()) {
        m_log->error("Charged particle TrackSegment found for this PID, but no unique Track is linked");
        continue;
      }
      auto id_particle = charged_particle_track.id();
      auto idx_paired  = std::make_pair(idx_coll, idx_pid);
      m_log->trace("  idx_pid={}  id_particle={}", idx_pid, id_particle);

      // insert in `particle_pids`
      auto it = particle_pids.find(id_particle);
      if(it == particle_pids.end())
        particle_pids.insert({id_particle, {idx_paired}});
      else
        it->second.push_back(idx_paired);
    }
  }

  // trace logging
  if(m_log->level() <= spdlog::level::trace) {
    m_log->trace("{:-<70}","Resulting `particle_pids` ");
    for(auto& [id_particle, idx_paired_list] : particle_pids) {
      m_log->trace("id_particle={}", id_particle);
      for(auto& [idx_coll, idx_pid] : idx_paired_list)
        m_log->trace("  (idx_coll, idx_pid) = ({}, {})", idx_coll, idx_pid);
    }
  }

  // --------------------------------------------------------------------------------


  // loop over charged particles, combine weights from the associated `CherenkovParticleID` objects
  // and create a merged output `CherenkovParticleID` object
  m_log->trace("{:-<70}","Begin Merging PID Objects ");
  for(auto& [id_particle, idx_paired_list] : particle_pids) {

    // trace logging
    m_log->trace("Charged Particle:");
    m_log->trace("  id = {}", id_particle);
    m_log->trace("  PID Hypotheses:");

    // create mutable output `CherenkovParticleID` object `out_pid`
    auto out_pid = out_pids->create();
    out_pid.setNpe(0.0);
    out_pid.setRefractiveIndex(0.0);
    out_pid.setPhotonEnergy(0.0);

    // define `pdg_2_out_hyp`: map of PDG => merged output hypothesis
    std::unordered_map< decltype(edm4eic::CherenkovParticleIDHypothesis::PDG), edm4eic::CherenkovParticleIDHypothesis > pdg_2_out_hyp;

    // merge each input `CherenkovParticleID` objects associated with this charged particle
    for(auto& [idx_coll, idx_pid] : idx_paired_list) {
      const auto& in_pid = in_pid_collections_list.at(idx_coll)->at(idx_pid);

      // logging
      m_log->trace("    Hypotheses for PID result (idx_coll, idx_pid) = ({}, {}):", idx_coll, idx_pid);
      Tools::PrintHypothesisTableHead(m_log,6);

      // merge scalar members
      out_pid.setNpe(out_pid.getNpe() + in_pid.getNpe()); // sum
      out_pid.setRefractiveIndex( out_pid.getRefractiveIndex() + in_pid.getNpe() * in_pid.getRefractiveIndex() ); // NPE-weighted average
      out_pid.setPhotonEnergy(    out_pid.getPhotonEnergy()    + in_pid.getNpe() * in_pid.getPhotonEnergy()    ); // NPE-weighted average

      // merge photon Cherenkov angles
      for(auto in_photon_vec : in_pid.getThetaPhiPhotons())
        out_pid.addToThetaPhiPhotons(in_photon_vec);

      // relate the charged particle
      if(!out_pid.getChargedParticle().isAvailable()) // only needs to be done once
        out_pid.setChargedParticle(in_pid.getChargedParticle());

      // merge PDG hypotheses, combining their weights and other members
      for(auto in_hyp : in_pid.getHypotheses()) {
        Tools::PrintHypothesisTableLine(m_log,in_hyp,6);
        auto out_hyp_it = pdg_2_out_hyp.find(in_hyp.PDG);
        if(out_hyp_it == pdg_2_out_hyp.end()) {
          edm4eic::CherenkovParticleIDHypothesis out_hyp;
          out_hyp.PDG    = in_hyp.PDG; // FIXME: no copy constructor?
          out_hyp.npe    = in_hyp.npe;
          out_hyp.weight = in_hyp.weight;
          pdg_2_out_hyp.insert({out_hyp.PDG,out_hyp});
        }
        else {
          auto& out_hyp = out_hyp_it->second;
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
      } // end `in_pid.getHypotheses()` loop, for this charged particle

    } // end `in_pid` loop, for this charged particle

    // finish computing averages of scalar members
    if(out_pid.getNpe() > 0) {
      out_pid.setRefractiveIndex( out_pid.getRefractiveIndex() / out_pid.getNpe() );
      out_pid.setPhotonEnergy(    out_pid.getPhotonEnergy()    / out_pid.getNpe() );
    }

    // append hypotheses
    for(auto [pdg,out_hyp] : pdg_2_out_hyp)
      out_pid.addToHypotheses(out_hyp);

    // logging: print merged hypothesis table
    m_log->trace("    => merged hypothesis weights:");
    Tools::PrintHypothesisTableHead(m_log,6);
    for(auto out_hyp : out_pid.getHypotheses())
      Tools::PrintHypothesisTableLine(m_log,out_hyp,6);

  } // end `particle_pids` loop over charged particles

  return out_pids;
}
