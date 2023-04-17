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
std::vector<eicrecon::ParticlesWithAssociation*> eicrecon::LinkParticleID::AlgorithmProcess(
    std::vector<const eicrecon::ParticlesWithAssociation*>& in_particles,
    std::vector<const edm4eic::CherenkovParticleID*>&       in_pids
    )
{
  // logging
  m_log->trace("{:=^70}"," call LinkParticleID::AlgorithmProcess ");

  // start output collections
  std::vector<eicrecon::ParticlesWithAssociation*> out_particles;

  // loop over `in_particles`
  for(auto& in_particle_with_assoc : in_particles) {

    // inputs and outputs
    auto in_recparts             = in_particle_with_assoc->particles();
    auto in_assocs               = in_particle_with_assoc->associations();
    std::vector<edm4eic::ReconstructedParticle*>     out_recparts;
    std::vector<edm4eic::MCRecoParticleAssociation*> out_assocs;

    // sanity check
    if(in_recparts.size() != in_assocs.size())
      m_log->warn("input ParticleWithAssociations has {} particles != {} associations", in_recparts.size(), in_assocs.size());

    // loop over `in_recparts`
    m_log->trace("{:-^70}"," Loop over reconstructed particles ");
    for(auto& in_recpart : in_recparts) {
      auto out_recpart = LinkParticle(*in_recpart, in_pids);
      out_recparts.push_back(new edm4eic::ReconstructedParticle(out_recpart));
    }

    // loop over `in_assocs`
    m_log->trace("{:-^70}"," Loop over reconstructed particle associations ");
    for(auto& in_assoc : in_assocs) {
      auto in_recpart = in_assoc->getRec();
      auto in_simpart = in_assoc->getSim();
      auto out_assoc  = in_assoc->clone(); // mutable copy
      auto out_recpart = LinkParticle(in_recpart, in_pids);
      out_assoc.setSim(in_simpart);
      out_assoc.setRec(out_recpart);
      out_assocs.push_back(new edm4eic::MCRecoParticleAssociation(out_assoc));
    }

    // append to output
    out_particles.push_back(new eicrecon::ParticlesWithAssociation(out_recparts,out_assocs));
  }

  return out_particles;
}


// LinkParticle function: link input `ReconstructedParticle` to a `ParticleID` from `in_pids`;
// returns a modified `ReconstructedParticle` that includes the `ParticleID` relations
//---------------------------------------------------------------------------
edm4eic::MutableReconstructedParticle eicrecon::LinkParticleID::LinkParticle(
    edm4eic::ReconstructedParticle                    in_particle,
    std::vector<const edm4eic::CherenkovParticleID*>& in_pids
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
    double match_dist;
    const edm4eic::CherenkovParticleID *pid;
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
  for(auto& in_pid : in_pids) {

    // get associated track's momentum angle
    auto in_track_segment = in_pid->getChargedParticle();
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
          prox_match_list.push_back(ProxMatch{match_dist, in_pid});

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
  auto in_pid_matched = closest_prox_match.pid;
  m_log->trace("  => best match: match_dist = {:<5.4}", closest_prox_match.match_dist);

  // convert PID hypotheses to edm4hep::ParticleID objects, sorted by likelihood
  auto out_pids = ConvertParticleID::ConvertToParticleIDs(*in_pid_matched, true);
  if(out_pids.size()==0) {
    m_log->error("found CherenkovParticleID object with no hypotheses");
    return out_particle;
  }

  // update reconstructed particle
  out_particle.setGoodnessOfPID(1); // FIXME: not used yet, aside from 0=noPID vs 1=hasPID
  out_particle.setParticleIDUsed(out_pids.at(0)); // highest likelihood is the first
  // for(auto& out_pid : out_pids) out_particle.addToParticleIDs(out_pid); // FIXME: cannot persistify 1-N relation?

  // logging
  m_log->trace("    {:.^50}"," PID result ");
  m_log->trace("      PID PDG vs. true PDG: {:>10} vs. {:<10}",
      out_particle.getParticleIDUsed().isAvailable() ? out_particle.getParticleIDUsed().getPDG() : 0,
      out_particle.getPDG()
      );
  m_log->trace("      Hypotheses (sorted):");
  Tools::PrintHypothesisTableHead(m_log, 8);
  for(auto out_pid : out_pids) // FIXME: loop through `out_particle.getParticleIDs()` (cf. 1-N persistify FIXME above)
    Tools::PrintHypothesisTableLine(m_log, out_pid, 8);
  m_log->trace("    {:'^50}","");

  return out_particle;

}
