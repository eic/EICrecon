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

  // loop over input collection, building `charged_particle_id_2_pids`, which maps a charged particle object ID
  // to the list of `CherenkovParticleID`s associated to it
  std::unordered_map<unsigned int, std::vector<const edm4eic::CherenkovParticleID*>> charged_particle_id_2_pids;
  for(const auto& in_pid : in_pids) {
    auto id         = in_pid->getChargedParticle().id();
    auto pid_map_it = charged_particle_id_2_pids.find(id);
    if(pid_map_it == charged_particle_id_2_pids.end())
      charged_particle_id_2_pids.insert({id,{in_pid}});
    else
      pid_map_it->second.push_back(in_pid);
  }

  // loop over charged particles, combine weights from the associated Cherenkov PIDs,
  // and create output `CherenkovParticleID` objects
  for(auto [id, in_particle_pids] : charged_particle_id_2_pids) {

    // logging
    m_log->trace("Charged Particle:");
    m_log->trace("  id = {}", id);
    m_log->trace("  PID Hypotheses:");

    // create mutable output objects
    edm4eic::MutableCherenkovParticleID out_pid{
      0,   // radiator (not used)
      0.0, // npe
      0.0, // refractiveIndex (not used, since radiator-dependent)
      0.0  // photonEnergy
    };
    edm4eic::MutableTrackSegment out_track_segment{
      0.0, // length
      0.0  // lengthError (not used)
    };
      
    // local lists, to support output
    std::unordered_map< // map of PDG to a merged output hypothesis
      decltype(edm4eic::CherenkovParticleIDHypothesis::PDG),
      edm4eic::CherenkovParticleIDHypothesis>
        pdg_2_out_hyp;
    std::vector<edm4eic::TrackPoint> out_track_points; // list of associated TrackPoints

    // merge `CherenkovParticleID` objects from each radiator to one: `out_pid`
    for(auto in_pid : in_particle_pids) {

      // logging
      m_log->trace("    radiator {} hypothesis weights:", in_pid->getRadiator());
      Tools::PrintHypothesisTableHead(m_log,6);

      // merge scalar members from each radiator
      out_pid.setNpe(out_pid.getNpe() + in_pid->getNpe()); // NPE is summed
      out_pid.setPhotonEnergy(in_pid->getNpe() * in_pid->getPhotonEnergy()); // photon energy is averaged (NPE-weighted)

      // include photon Cherenkov angles from each radiator
      for(auto in_photon_vec : in_pid->getThetaPhiPhotons())
        out_pid.addToThetaPhiPhotons(in_photon_vec);

      // merge PDG hypotheses from each radiator, combining their weights and other members
      for(auto in_hyp : in_pid->getHypotheses()) {
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

        // merge TrackSegments
        auto in_track_segment = in_pid->getChargedParticle();
        if(in_track_segment.isAvailable()) {
          out_track_segment.setLength(out_track_segment.getLength() + in_track_segment.getLength());
          for(auto in_track_point : in_track_segment.getPoints())
            out_track_points.push_back(in_track_point);
        }

        // merge input sensor hits
        // TODO

      } // end `in_pid->getHypotheses()` loop, for this charged particle
    } // end `in_particle_pids` loop, for this charged particle

    // finish computing averages of scalar members
    if(out_pid.getNpe() > 0)
      out_pid.setPhotonEnergy(out_pid.getPhotonEnergy() / out_pid.getNpe());

    // sort trackpoints by time, and add them to `out_track_segment`
    std::sort(
        out_track_points.begin(),
        out_track_points.end(),
        [] (edm4eic::TrackPoint& a, edm4eic::TrackPoint& b) { return a.time < b.time; }
        );
    for(auto out_track_point : out_track_points)
      out_track_segment.addToPoints(out_track_point);

    // add remaining non-scalar members and relations to `out_pid`
    for(auto [pdg,out_hyp] : pdg_2_out_hyp)
      out_pid.addToHypotheses(out_hyp);
    out_pid.setChargedParticle(out_track_segment);

    // logging: print merged hypothesis table
    m_log->trace("    => merged hypothesis weights:");
    Tools::PrintHypothesisTableHead(m_log,6);
    for(auto out_hyp : out_pid.getHypotheses())
      Tools::PrintHypothesisTableLine(m_log,out_hyp,6);

    // store in `out_pids` output collection
    out_pids.push_back(new edm4eic::CherenkovParticleID(out_pid)); // force immutable

  } // end `charged_particle_id_2_pids` loop

  return out_pids;
}
