// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023, Christopher Dilks

#include "MergeParticleID.h"

#include <algorithms/logger.h>
#include <edm4eic/CherenkovParticleIDHypothesis.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4hep/Vector2f.h>
#include <fmt/core.h>
#include <podio/ObjectID.h>
#include <podio/RelationRange.h>
#include <cstddef>
#include <gsl/pointers>
#include <stdexcept>
#include <unordered_map>
#include <utility>

#include "algorithms/pid/MergeParticleIDConfig.h"
#include "algorithms/pid/Tools.h"

namespace eicrecon {

void MergeParticleID::init() { debug() << m_cfg << endmsg; }

void MergeParticleID::process(const MergeParticleID::Input& input,
                              const MergeParticleID::Output& output) const {

  const auto [in_pid_collections_list] = input;
  auto [out_pids]                      = output;

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
  std::unordered_map<decltype(podio::ObjectID::index),
                     std::vector<std::pair<std::size_t, std::size_t>>>
      particle_pids;
  trace("{:-<70}", "Build `particle_pids` indexing data structure ");

  // loop over list of PID collections
  for (std::size_t idx_coll = 0; idx_coll < in_pid_collections_list.size(); idx_coll++) {
    const auto& in_pid_collection = in_pid_collections_list.at(idx_coll);
    trace("idx_col={}", idx_coll);

    // loop over this PID collection
    for (std::size_t idx_pid = 0; idx_pid < in_pid_collection->size(); idx_pid++) {
      // make the index pair
      const auto& in_pid                         = in_pid_collection->at(idx_pid);
      const auto& charged_particle_track_segment = in_pid.getChargedParticle();
      if (!charged_particle_track_segment.isAvailable()) {
        error("PID object found with no charged particle");
        continue;
      }
      auto id_particle = charged_particle_track_segment.getObjectID().index;
      auto idx_paired  = std::make_pair(idx_coll, idx_pid);
      trace("  idx_pid={}  id_particle={}", idx_pid, id_particle);

      // insert in `particle_pids`
      auto it = particle_pids.find(id_particle);
      if (it == particle_pids.end()) {
        particle_pids.insert({id_particle, {idx_paired}});
      } else {
        it->second.push_back(idx_paired);
      }
    }
  }

  // trace logging
  if (level() <= algorithms::LogLevel::kTrace) {
    trace("{:-<70}", "Resulting `particle_pids` ");
    for (auto& [id_particle, idx_paired_list] : particle_pids) {
      trace("id_particle={}", id_particle);
      for (auto& [idx_coll, idx_pid] : idx_paired_list) {
        trace("  (idx_coll, idx_pid) = ({}, {})", idx_coll, idx_pid);
      }
    }
  }

  // --------------------------------------------------------------------------------

  // loop over charged particles, combine weights from the associated `CherenkovParticleID` objects
  // and create a merged output `CherenkovParticleID` object
  trace("{:-<70}", "Begin Merging PID Objects ");
  for (auto& [id_particle, idx_paired_list] : particle_pids) {

    // trace logging
    trace("Charged Particle:");
    trace("  id = {}", id_particle);
    trace("  PID Hypotheses:");

    // create mutable output `CherenkovParticleID` object `out_pid`
    auto out_pid                                            = out_pids->create();
    decltype(edm4eic::CherenkovParticleIDData::npe) out_npe = 0.0;
    decltype(edm4eic::CherenkovParticleIDData::refractiveIndex) out_refractiveIndex = 0.0;
    decltype(edm4eic::CherenkovParticleIDData::photonEnergy) out_photonEnergy       = 0.0;

    // define `pdg_2_out_hyp`: map of PDG => merged output hypothesis
    std::unordered_map<decltype(edm4eic::CherenkovParticleIDHypothesis::PDG),
                       edm4eic::CherenkovParticleIDHypothesis>
        pdg_2_out_hyp;

    // merge each input `CherenkovParticleID` objects associated with this charged particle
    for (auto& [idx_coll, idx_pid] : idx_paired_list) {
      const auto& in_pid = in_pid_collections_list.at(idx_coll)->at(idx_pid);

      // logging
      trace("    Hypotheses for PID result (idx_coll, idx_pid) = ({}, {}):", idx_coll, idx_pid);
      trace(Tools::HypothesisTableHead(6));

      // merge scalar members
      out_npe += in_pid.getNpe();                                           // sum
      out_refractiveIndex += in_pid.getNpe() * in_pid.getRefractiveIndex(); // NPE-weighted average
      out_photonEnergy += in_pid.getNpe() * in_pid.getPhotonEnergy();       // NPE-weighted average

      // merge photon Cherenkov angles
      for (auto in_photon_vec : in_pid.getThetaPhiPhotons()) {
        out_pid.addToThetaPhiPhotons(in_photon_vec);
      }

      // relate the charged particle
      if (!out_pid.getChargedParticle().isAvailable()) { // only needs to be done once
        out_pid.setChargedParticle(in_pid.getChargedParticle());
      }

      // merge PDG hypotheses, combining their weights and other members
      for (auto in_hyp : in_pid.getHypotheses()) {
        trace(Tools::HypothesisTableLine(in_hyp, 6));
        auto out_hyp_it = pdg_2_out_hyp.find(in_hyp.PDG);
        if (out_hyp_it == pdg_2_out_hyp.end()) {
          edm4eic::CherenkovParticleIDHypothesis out_hyp;
          out_hyp.PDG    = in_hyp.PDG; // FIXME: no copy constructor?
          out_hyp.npe    = in_hyp.npe;
          out_hyp.weight = in_hyp.weight;
          pdg_2_out_hyp.insert({out_hyp.PDG, out_hyp});
        } else {
          auto& out_hyp = out_hyp_it->second;
          out_hyp.npe += in_hyp.npe;
          // combine hypotheses' weights
          switch (m_cfg.mergeMode) {
          case MergeParticleIDConfig::kAddWeights:
            out_hyp.weight += in_hyp.weight;
            break;
          case MergeParticleIDConfig::kMultiplyWeights:
            out_hyp.weight *= in_hyp.weight;
            break;
          default:
            throw std::runtime_error(
                "unknown MergeParticleIDConfig::mergeMode setting; weights not combined");
          }
        }
      } // end `in_pid.getHypotheses()` loop, for this charged particle

    } // end `in_pid` loop, for this charged particle

    // finish computing averages of scalar members
    out_pid.setNpe(out_npe);
    if (out_npe > 0) {
      out_pid.setRefractiveIndex(out_refractiveIndex / out_npe);
      out_pid.setPhotonEnergy(out_photonEnergy / out_npe);
    }

    // append hypotheses
    for (auto [pdg, out_hyp] : pdg_2_out_hyp) {
      out_pid.addToHypotheses(out_hyp);
    }

    // logging: print merged hypothesis table
    trace("    => merged hypothesis weights:");
    trace(Tools::HypothesisTableHead(6));
    for (auto out_hyp : out_pid.getHypotheses()) {
      trace(Tools::HypothesisTableLine(out_hyp, 6));
    }

  } // end `particle_pids` loop over charged particles
}

} // namespace eicrecon
