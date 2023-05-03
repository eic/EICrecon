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


// AlgorithmProcess: for ReconstructedParticleCollection
//---------------------------------------------------------------------------
std::unique_ptr<edm4eic::ReconstructedParticleCollection> eicrecon::LinkParticleID::AlgorithmProcess(
    const edm4eic::CherenkovParticleIDCollection*   in_pids,
    const edm4eic::ReconstructedParticleCollection* in_particles
    )
{
  // logging
  m_log->trace("{:=^70}"," call LinkParticleID::AlgorithmProcess for ReconstructedParticleCollection ");

  // start output collection
  auto out_particles = std::make_unique<edm4eic::ReconstructedParticleCollection>();

  // loop over input particles, calling LinkParticle on each to link PID
  m_log->trace("{:-^70}"," Loop over reconstructed particles ");
  for(const auto& in_particle : *in_particles) {
    auto out_particle = LinkParticle(in_pids, in_particle);
    out_particles->push_back(out_particle);
  }

  return out_particles;
}


// AlgorithmProcess: for MCRecoParticleAssociationCollection
//---------------------------------------------------------------------------
std::unique_ptr<edm4eic::MCRecoParticleAssociationCollection> eicrecon::LinkParticleID::AlgorithmProcess(
    const edm4eic::CherenkovParticleIDCollection*       in_pids,
    const edm4eic::MCRecoParticleAssociationCollection* in_assocs
    )
{
  // logging
  m_log->trace("{:=^70}"," call LinkParticleID::AlgorithmProcess for MCRecoParticleAssociationCollection ");

  // start output collection
  auto out_assocs = std::make_unique<edm4eic::MCRecoParticleAssociationCollection>();

  // loop over input associations, calling LinkParticle on each to link PID
  m_log->trace("{:-^70}"," Loop over reconstructed particle associations ");
  for(const auto& in_assoc : *in_assocs) {
    auto out_assoc   = in_assoc.clone();
    auto out_recpart = LinkParticle(in_pids, in_assoc.getRec());
    out_assoc.setRec(out_recpart);
    out_assocs->push_back(out_assoc);
  }

  return out_assocs;
}


// LinkParticle function: link input `ReconstructedParticle` to a `ParticleID` from `in_pids`;
// returns a modified `ReconstructedParticle` that includes the `ParticleID` relations
//---------------------------------------------------------------------------
edm4eic::MutableReconstructedParticle eicrecon::LinkParticleID::LinkParticle(
    const edm4eic::CherenkovParticleIDCollection* in_pids,
    edm4eic::ReconstructedParticle                in_particle
    )
{
  // make a mutable copy, and reset its stored PID info
  auto out_particle = in_particle.clone();
  out_particle.setGoodnessOfPID(0);

  // skip this particle, if neutral
  if(std::abs(in_particle.getCharge()) < 0.001)
    return out_particle;

  // list of candidate matches
  struct ProxMatch {
    double      match_dist;
    std::size_t pid_idx;
  };
  std::vector<ProxMatch> prox_match_list;

  // get input reconstructed particle momentum angles
  auto in_particle_p   = in_particle.getMomentum();
  auto in_particle_eta = edm4hep::utils::eta(in_particle_p);
  auto in_particle_phi = edm4hep::utils::angleAzimuthal(in_particle_p);
  m_log->trace("Input particle: PDG = {:<8} (eta,phi) = ( {:>5.4}, {:>5.4} deg )",
      in_particle.getPDG(),
      in_particle_eta,
      in_particle_phi / dd4hep::degree
      );

  // loop over input PID objects
  for(std::size_t in_pid_idx = 0; in_pid_idx < in_pids->size(); in_pid_idx++) {
    auto in_pid = in_pids->at(in_pid_idx);

    // get associated track's momentum angle
    auto in_track_segment = in_pid.getChargedParticle();
    if(in_track_segment.isAvailable()) {
      m_log->trace("Candidate tracks with PID:");
      decltype(edm4eic::TrackPoint::momentum) in_track_p{0.0, 0.0, 0.0};
      if(in_track_segment.points_size() > 0) {

        // get averge momentum direction of the track's TrackPoints
        for(auto& in_track_point : in_track_segment.getPoints())
          in_track_p = in_track_p + ( in_track_point.momentum / in_track_segment.points_size() );

        // get momentum angles, and the (eta,phi)-distance from the reconstructed particle
        auto in_track_eta = edm4hep::utils::eta(in_track_p);
        auto in_track_phi = edm4hep::utils::angleAzimuthal(in_track_p);
        auto match_dist   = std::hypot(
            in_particle_eta - in_track_eta,
            in_particle_phi - in_track_phi
            );

        // check if the match is close enough, within user-specified tolerances
        auto match_is_close =
          std::abs(in_particle_eta - in_track_eta) < m_cfg.etaTolerance &&
          std::abs(in_particle_phi - in_track_phi) < m_cfg.phiTolerance;
        if(match_is_close)
          prox_match_list.push_back(ProxMatch{match_dist, in_pid_idx});

        // logging
        m_log->trace("  - (eta,phi) = ( {:>5.4}, {:>5.4} deg ),  match_dist = {:<5.4}{}",
            in_track_eta,
            in_track_phi / dd4hep::degree,
            match_dist,
            match_is_close ? " => CLOSE!" : ""
            );
      }
      else m_log->warn("this PID's track has no points");
    }
    else {
      m_log->warn("no charged particle associated with PID");
      return out_particle;
    }

  } // end loop over input PID objects

  // if no match, proceed to next particle
  if(prox_match_list.size() == 0)
    return out_particle;

  // choose the closest matching PID object corresponding to this input reconstructed particle
  auto closest_prox_match = *std::min_element(
      prox_match_list.begin(),
      prox_match_list.end(),
      [] (ProxMatch a, ProxMatch b) { return a.match_dist < b.match_dist; }
      );
  auto in_pid_matched = in_pids->at(closest_prox_match.pid_idx);
  m_log->trace("  => best match: match_dist = {:<5.4}", closest_prox_match.match_dist);

  // convert PID hypotheses to edm4hep::ParticleID objects, sorted by likelihood
  auto out_pids = ConvertParticleID::ConvertToParticleIDs(in_pid_matched, true);
  if(out_pids->size()==0) {
    m_log->error("found CherenkovParticleID object with no hypotheses");
    return out_particle;
  }

  // relate matched ParticleID objects to output particle
  for(const auto& out_pid : *out_pids)
    out_particle.addToParticleIDs(out_pid);
  out_particle.setParticleIDUsed(out_pids->at(0)); // highest likelihood is the first // FIXME: any consensus that this is what we want?
  out_particle.setGoodnessOfPID(1); // FIXME: not used yet, aside from 0=noPID vs 1=hasPID

  // logging
  m_log->trace("    {:.^50}"," PID result ");
  m_log->trace("      PID PDG vs. true PDG: {:>10} vs. {:<10}",
      out_particle.getParticleIDUsed().isAvailable() ? out_particle.getParticleIDUsed().getPDG() : 0,
      out_particle.getPDG()
      );
  m_log->trace("      Hypotheses (sorted):");
  Tools::PrintHypothesisTableHead(m_log, 8);
  for(auto out_pid : out_particle.getParticleIDs())
    Tools::PrintHypothesisTableLine(m_log, out_pid, 8);
  m_log->trace("    {:'^50}","");

  return out_particle;

}
