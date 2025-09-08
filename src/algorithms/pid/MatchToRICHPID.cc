// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 - 2024, Sylvester Joosten, Wouter Deconinck, Dmitry Romanov, Christopher Dilks, Dmitry Kalinkin

#include "MatchToRICHPID.h"

#include <edm4eic/TrackPoint.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <fmt/core.h>
#include <podio/ObjectID.h>
#include <podio/RelationRange.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <gsl/pointers>
#include <map>
#include <vector>

#include "algorithms/pid/ConvertParticleID.h"
#include "algorithms/pid/MatchToRICHPIDConfig.h"

namespace eicrecon {

void MatchToRICHPID::init() {}

void MatchToRICHPID::process(const MatchToRICHPID::Input& input,
                             const MatchToRICHPID::Output& output) const {
  const auto [parts_in, assocs_in, drich_cherenkov_pid] = input;
  auto [parts_out, assocs_out, pids]                    = output;

  for (auto part_in : *parts_in) {
    auto part_out = part_in.clone();

    // link Cherenkov PID objects
    auto success = linkCherenkovPID(part_out, *drich_cherenkov_pid, *pids);
    if (success) {
      trace("Previous PDG vs. CherenkovPID PDG: {:>10} vs. {:<10}", part_in.getPDG(),
            part_out.getParticleIDUsed().isAvailable() ? part_out.getParticleIDUsed().getPDG() : 0);
    }

    for (auto assoc_in : *assocs_in) {
      if (assoc_in.getRec() == part_in) {
        auto assoc_out = assoc_in.clone();
        assoc_out.setRec(part_out);
        assocs_out->push_back(assoc_out);
      }
    }

    parts_out->push_back(part_out);
  }
}

/* link PID objects to input particle
     * - finds `CherenkovParticleID` object in `in_pids` associated to particle `in_part`
     *   by proximity matching to the associated track
     * - converts this `CherenkovParticleID` object's PID hypotheses to `ParticleID` objects,
     *   relates them to `in_part`, and adds them to the collection `out_pids` for persistency
     * - returns `true` iff PID objects were found and linked
     */
bool MatchToRICHPID::linkCherenkovPID(edm4eic::MutableReconstructedParticle& in_part,
                                      const edm4eic::CherenkovParticleIDCollection& in_pids,
                                      edm4hep::ParticleIDCollection& out_pids) const {

  // skip this particle, if neutral
  if (std::abs(in_part.getCharge()) < 0.001) {
    return false;
  }

  // structure to store list of candidate matches
  struct ProxMatch {
    double match_dist;
    std::size_t pid_idx;
  };
  std::vector<ProxMatch> prox_match_list;

  // get input reconstructed particle momentum angles
  auto in_part_p   = in_part.getMomentum();
  auto in_part_eta = edm4hep::utils::eta(in_part_p);
  auto in_part_phi = edm4hep::utils::angleAzimuthal(in_part_p);
  trace("Input particle: (eta,phi) = ( {:>5.4}, {:>5.4} deg )", in_part_eta,
        in_part_phi * 180.0 / M_PI);

  // loop over input CherenkovParticleID objects
  for (std::size_t in_pid_idx = 0; in_pid_idx < in_pids.size(); in_pid_idx++) {
    auto in_pid = in_pids.at(in_pid_idx);

    // get charged particle track associated to this CherenkovParticleID object
    auto in_track = in_pid.getChargedParticle();
    if (!in_track.isAvailable()) {
      error("found CherenkovParticleID object with no chargedParticle");
      return false;
    }
    if (in_track.points_size() == 0) {
      error("found chargedParticle for CherenkovParticleID, but it has no TrackPoints");
      return false;
    }

    // get average momentum direction of the track's TrackPoints
    decltype(edm4eic::TrackPoint::momentum) in_track_p{0.0, 0.0, 0.0};
    for (const auto& in_track_point : in_track.getPoints()) {
      in_track_p = in_track_p + (in_track_point.momentum / in_track.points_size());
    }
    auto in_track_eta = edm4hep::utils::eta(in_track_p);
    auto in_track_phi = edm4hep::utils::angleAzimuthal(in_track_p);

    // calculate dist(eta,phi)
    auto match_dist = std::hypot(in_part_eta - in_track_eta, in_part_phi - in_track_phi);

    // check if the match is close enough: within user-specified tolerances
    auto match_is_close = std::abs(in_part_eta - in_track_eta) < m_cfg.etaTolerance &&
                          std::abs(in_part_phi - in_track_phi) < m_cfg.phiTolerance;
    if (match_is_close) {
      prox_match_list.push_back(ProxMatch{match_dist, in_pid_idx});
    }

    // logging
    trace("  - (eta,phi) = ( {:>5.4}, {:>5.4} deg ),  match_dist = {:<5.4}{}", in_track_eta,
          in_track_phi * 180.0 / M_PI, match_dist, match_is_close ? " => CLOSE!" : "");

  } // end loop over input CherenkovParticleID objects

  // check if at least one match was found
  if (prox_match_list.empty()) {
    trace("  => no matching CherenkovParticleID found for this particle");
    return false;
  }

  // choose the closest matching CherenkovParticleID object corresponding to this input reconstructed particle
  auto closest_prox_match =
      *std::min_element(prox_match_list.begin(), prox_match_list.end(),
                        [](ProxMatch a, ProxMatch b) { return a.match_dist < b.match_dist; });
  auto in_pid_matched = in_pids.at(closest_prox_match.pid_idx);
  trace("  => best match: match_dist = {:<5.4} at idx = {}", closest_prox_match.match_dist,
        closest_prox_match.pid_idx);

  // convert `CherenkovParticleID` object's hypotheses => set of `ParticleID` objects
  auto out_pid_index_map = ConvertParticleID::ConvertToParticleIDs(in_pid_matched, out_pids, true);
  if (out_pid_index_map.empty()) {
    error("found CherenkovParticleID object with no hypotheses");
    return false;
  }

  // relate matched ParticleID objects to output particle
  for (const auto& [out_pids_index, out_pids_id] : out_pid_index_map) {
    const auto& out_pid = out_pids.at(out_pids_index);
    if (out_pid.getObjectID().index != static_cast<int>(out_pids_id)) { // sanity check
      error("indexing error in `edm4eic::ParticleID` collection");
      return false;
    }
    in_part.addToParticleIDs(out_pid);
  }
  in_part.setParticleIDUsed(in_part.getParticleIDs().at(0)); // highest likelihood is the first
  in_part.setGoodnessOfPID(1); // FIXME: not used yet, aside from 0=noPID vs 1=hasPID

  // trace logging
  trace("    {:.^50}", " PID results ");
  trace("      Hypotheses (sorted):");
  for (auto hyp : in_part.getParticleIDs()) {
    float npe =
        hyp.parameters_size() > 0 ? hyp.getParameters(0) : -1; // assume NPE is the first parameter
    trace("{:{}}{:>6}  {:>10.8}  {:>10.8}", "", 8, hyp.getPDG(), hyp.getLikelihood(), npe);
  }
  trace("    {:'^50}", "");

  return true;
}
} // namespace eicrecon
