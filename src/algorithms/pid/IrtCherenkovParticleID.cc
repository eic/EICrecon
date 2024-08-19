// Copyright 2023, Christopher Dilks, adapted from Alexander Kiselev's Juggler implementation `IRTAlgorithm`
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "IrtCherenkovParticleID.h"

#include <IRT/ChargedParticle.h>
#include <IRT/CherenkovPID.h>
#include <IRT/OpticalPhoton.h>
#include <IRT/RadiatorHistory.h>
#include <IRT/SinglePDF.h>
#include <TString.h>
#include <TVector3.h>
#include <edm4eic/CherenkovParticleIDHypothesis.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/TrackPoint.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <edm4hep/Vector2f.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <podio/ObjectID.h>
#include <podio/RelationRange.h>
#include <spdlog/common.h>
#include <algorithm>
#include <cstddef>
#include <functional>
#include <gsl/pointers>
#include <iterator>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

#include "algorithms/pid/IrtCherenkovParticleIDConfig.h"
#include "algorithms/pid/Tools.h"

namespace eicrecon {

void IrtCherenkovParticleID::init(
    CherenkovDetectorCollection*     irt_det_coll,
    std::shared_ptr<spdlog::logger>& logger
    )
{
  // members
  m_irt_det_coll = irt_det_coll;
  m_log          = logger;

  // print the configuration parameters
  m_cfg.Print(m_log, spdlog::level::debug);

  // inform the user if a cheat mode is enabled
  m_cfg.PrintCheats(m_log);

  // extract the the relevant `CherenkovDetector`, set to `m_irt_det`
  auto& detectors = m_irt_det_coll->GetDetectors();
  if(detectors.size() == 0)
    throw std::runtime_error("No CherenkovDetectors found in input collection `irt_det_coll`");
  if(detectors.size() > 1)
    m_log->warn("IrtCherenkovParticleID currently only supports 1 CherenkovDetector at a time; taking the first");
  auto this_detector = *detectors.begin();
  m_det_name         = this_detector.first;
  m_irt_det          = this_detector.second;
  m_log->debug("Initializing IrtCherenkovParticleID algorithm for CherenkovDetector '{}'", m_det_name);

  // readout decoding
  m_cell_mask = m_irt_det->GetReadoutCellMask();
  m_log->debug("readout cellMask = {:#X}", m_cell_mask);

  // rebin refractive index tables to have `m_cfg.numRIndexBins` bins
  m_log->trace("Rebinning refractive index tables to have {} bins",m_cfg.numRIndexBins);
  for(auto [rad_name,irt_rad] : m_irt_det->Radiators()) {
    auto ri_lookup_table_orig = irt_rad->m_ri_lookup_table;
    irt_rad->m_ri_lookup_table.clear();
    irt_rad->m_ri_lookup_table = Tools::ApplyFineBinning( ri_lookup_table_orig, m_cfg.numRIndexBins );
    // m_log->trace("- {}", rad_name);
    // for(auto [energy,rindex] : irt_rad->m_ri_lookup_table) m_log->trace("  {:>5} eV   {:<}", energy, rindex);
  }

  // build `m_pid_radiators`, the list of radiators to use for PID
  m_log->debug("Obtain List of Radiators:");
  for(auto [rad_name,irt_rad] : m_irt_det->Radiators()) {
    if(rad_name!="Filter") {
      m_pid_radiators.insert({ std::string(rad_name), irt_rad });
      m_log->debug("- {}", rad_name.Data());
    }
  }

  // check radiators' configuration, and pass it to `m_irt_det`'s radiators
  for(auto [rad_name,irt_rad] : m_pid_radiators) {
    // find `cfg_rad`, the associated `IrtCherenkovParticleIDConfig` radiator
    auto cfg_rad_it = m_cfg.radiators.find(rad_name);
    if(cfg_rad_it != m_cfg.radiators.end()) {
      auto cfg_rad = cfg_rad_it->second;
      // pass `cfg_rad` params to `irt_rad`, the IRT radiator
      irt_rad->m_ID = Tools::GetRadiatorID(std::string(rad_name));
      irt_rad->m_AverageRefractiveIndex = cfg_rad.referenceRIndex;
      irt_rad->SetReferenceRefractiveIndex(cfg_rad.referenceRIndex);
      if(cfg_rad.attenuation>0)
        irt_rad->SetReferenceAttenuationLength(cfg_rad.attenuation);
      if(cfg_rad.smearing>0) {
        if(cfg_rad.smearingMode=="uniform")
          irt_rad->SetUniformSmearing(cfg_rad.smearing);
        else if(cfg_rad.smearingMode=="gaussian")
          irt_rad->SetGaussianSmearing(cfg_rad.smearing);
        else
          m_log->error("Unknown smearing mode '{}' for {} radiator", cfg_rad.smearingMode, rad_name);
      }
    }
    else
      m_log->error("Cannot find radiator '{}' in IrtCherenkovParticleIDConfig instance", rad_name);
  }

  // get PDG info for the particles we want to identify in PID
  m_log->debug("List of particles for PID:");
  for(auto pdg : m_cfg.pdgList) {
    auto mass = m_particleSvc.particle(pdg).mass;
    m_pdg_mass.insert({ pdg, mass });
    m_log->debug("  {:>8}  M={} GeV", pdg, mass);
  }

}

void IrtCherenkovParticleID::process(
    const IrtCherenkovParticleID::Input& input,
    const IrtCherenkovParticleID::Output& output) const
{
  const auto [in_aerogel_tracks, in_gas_tracks, in_merged_tracks, in_raw_hits, in_hit_assocs] = input;
  auto [out_aerogel_particleIDs, out_gas_particleIDs] = output;

  // logging
  m_log->trace("{:=^70}"," call IrtCherenkovParticleID::AlgorithmProcess ");
  m_log->trace("number of raw sensor hits: {}", in_raw_hits->size());
  m_log->trace("number of raw sensor hit with associated photons: {}", in_hit_assocs->size());

  std::map<std::string, const edm4eic::TrackSegmentCollection*> in_charged_particles{
    {"Aerogel", in_aerogel_tracks},
    {"Gas", in_gas_tracks},
    {"Merged", in_merged_tracks},
  };

  // start output collections
  std::map<std::string, edm4eic::CherenkovParticleIDCollection*> out_cherenkov_pids{
    {"Aerogel", out_aerogel_particleIDs},
    {"Gas", out_gas_particleIDs}
  };

  // check `in_charged_particles`: each radiator should have the same number of TrackSegments
  std::unordered_map<std::size_t, std::size_t> in_charged_particle_size_distribution;
  for(const auto& [rad_name, in_charged_particle] : in_charged_particles) {
    ++in_charged_particle_size_distribution[in_charged_particle->size()];
  }
  if (in_charged_particle_size_distribution.size() != 1) {
    std::vector<size_t> in_charged_particle_sizes;
    std::transform(in_charged_particles.begin(), in_charged_particles.end(),
      std::back_inserter(in_charged_particle_sizes),
      [](const auto& in_charged_particle) { return in_charged_particle.second->size(); });
    m_log->error("radiators have differing numbers of TrackSegments {}", fmt::join(in_charged_particle_sizes, ", "));
    return;
  }

  // loop over charged particles ********************************************
  m_log->trace("{:#<70}","### CHARGED PARTICLES ");
  std::size_t num_charged_particles = in_charged_particle_size_distribution.begin()->first;
  for(long i_charged_particle=0; i_charged_particle<num_charged_particles; i_charged_particle++) {
    m_log->trace("{:-<70}", fmt::format("--- charged particle #{} ", i_charged_particle));

    // start an `irt_particle`, for `IRT`
    auto irt_particle = std::make_unique<ChargedParticle>();

    // loop over radiators
    for(auto [rad_name,irt_rad] : m_pid_radiators) {

      // get the `charged_particle` for this radiator
      auto charged_particle_list_it = in_charged_particles.find(rad_name);
      if(charged_particle_list_it == in_charged_particles.end()) {
        m_log->error("Cannot find radiator '{}' in `in_charged_particles`", rad_name);
        continue;
      }
      const auto *charged_particle_list = charged_particle_list_it->second;
      auto charged_particle      = charged_particle_list->at(i_charged_particle);

      // set number of bins for this radiator and charged particle
      if(charged_particle.points_size()==0) {
        m_log->trace("No propagated track points in radiator '{}'", rad_name);
        continue;
      }
      irt_rad->SetTrajectoryBinCount(charged_particle.points_size() - 1);

      // start a new IRT `RadiatorHistory`
      // - must be a raw pointer for `irt` compatibility
      // - it will be destroyed when `irt_particle` is destroyed
      auto *irt_rad_history = new RadiatorHistory();
      irt_particle->StartRadiatorHistory({ irt_rad, irt_rad_history });

      // loop over `TrackPoint`s of this `charged_particle`, adding each to the IRT radiator
      irt_rad->ResetLocations();
      m_log->trace("TrackPoints in '{}' radiator:", rad_name);
      for(const auto& point : charged_particle.getPoints()) {
        TVector3 position = Tools::PodioVector3_to_TVector3(point.position);
        TVector3 momentum = Tools::PodioVector3_to_TVector3(point.momentum);
        irt_rad->AddLocation(position, momentum);
        Tools::PrintTVector3(m_log, " point: x", position);
        Tools::PrintTVector3(m_log, "        p", momentum);
      }


      // loop over raw hits ***************************************************
      m_log->trace("{:#<70}","### SENSOR HITS ");
      for(const auto& raw_hit : *in_raw_hits) {

        // get MC photon(s), typically only used by cheat modes or trace logging
        // - loop over `in_hit_assocs`, searching for the matching hit association
        // - will not exist for noise hits
        edm4hep::MCParticle mc_photon;
        bool mc_photon_found = false;
        if(m_cfg.cheatPhotonVertex || m_cfg.cheatTrueRadiator) {
          for(const auto& hit_assoc : *in_hit_assocs) {
            if(hit_assoc.getRawHit().isAvailable()) {
              if(hit_assoc.getRawHit().id() == raw_hit.id()) {
#if EDM4EIC_VERSION_MAJOR >= 6
                mc_photon = hit_assoc.getSimHit().getMCParticle();
#else
                // hit association found, get the MC photon and break the loop
                if(hit_assoc.simHits_size() > 0) {
                  mc_photon = hit_assoc.getSimHits(0).getMCParticle();
#endif
                  mc_photon_found = true;
                  if(mc_photon.getPDG() != -22)
                    m_log->warn("non-opticalphoton hit: PDG = {}",mc_photon.getPDG());
#if EDM4EIC_VERSION_MAJOR >= 6
#else
                }
                else if(m_cfg.CheatModeEnabled())
                  m_log->error("cheat mode enabled, but no MC photons provided");
#endif
                break;
              }
            }
          }
        }

        // cheat mode, for testing only: use MC photon to get the actual radiator
        if(m_cfg.cheatTrueRadiator && mc_photon_found) {
          auto vtx    = Tools::PodioVector3_to_TVector3(mc_photon.getVertex());
          auto mc_rad = m_irt_det->GuessRadiator(vtx, vtx); // assume IP is at (0,0,0)
          if(mc_rad != irt_rad) continue; // skip this photon, if not from radiator `irt_rad`
          Tools::PrintTVector3(m_log, fmt::format("cheat: radiator '{}' determined from photon vertex", rad_name), vtx);
        }

        // get sensor and pixel info
        // FIXME: signal and timing cuts (ADC, TDC, ToT, ...)
        auto     cell_id   = raw_hit.getCellID();
        uint64_t sensor_id = cell_id & m_cell_mask;
        TVector3 pixel_pos = m_irt_det->m_ReadoutIDToPosition(cell_id);

        // trace logging
        if(m_log->level() <= spdlog::level::trace) {
          m_log->trace("cell_id={:#X}  sensor_id={:#X}", cell_id, sensor_id);
          Tools::PrintTVector3(m_log, "pixel position", pixel_pos);
          if(mc_photon_found) {
            TVector3 mc_endpoint = Tools::PodioVector3_to_TVector3(mc_photon.getEndpoint());
            Tools::PrintTVector3(m_log, "photon endpoint", mc_endpoint);
            m_log->trace("{:>30} = {}", "dist( pixel,  photon )", (pixel_pos  - mc_endpoint).Mag());
          }
          else m_log->trace("  no MC photon found; probably a noise hit");
        }

        // start new IRT photon
        auto *irt_sensor = m_irt_det->m_PhotonDetectors[0]; // NOTE: assumes one sensor type
        auto *irt_photon = new OpticalPhoton(); // new raw pointer; it will also be destroyed when `irt_particle` is destroyed
        irt_photon->SetVolumeCopy(sensor_id);
        irt_photon->SetDetectionPosition(pixel_pos);
        irt_photon->SetPhotonDetector(irt_sensor);
        irt_photon->SetDetected(true);

        // cheat mode: get photon vertex info from MC truth
        if((m_cfg.cheatPhotonVertex || m_cfg.cheatTrueRadiator) && mc_photon_found) {
          irt_photon->SetVertexPosition(Tools::PodioVector3_to_TVector3(mc_photon.getVertex()));
          irt_photon->SetVertexMomentum(Tools::PodioVector3_to_TVector3(mc_photon.getMomentum()));
        }

        // cheat mode: retrieve a refractive index estimate; it is not exactly the one, which
        // was used in GEANT, but should be very close
        if(m_cfg.cheatPhotonVertex) {
          double ri;
          auto mom    = 1e9 * irt_photon->GetVertexMomentum().Mag();
          auto ri_set = Tools::GetFinelyBinnedTableEntry(irt_rad->m_ri_lookup_table, mom, &ri);
          if(ri_set) {
            irt_photon->SetVertexRefractiveIndex(ri);
            m_log->trace("{:>30} = {}", "refractive index", ri);
          }
          else
            m_log->warn("Tools::GetFinelyBinnedTableEntry failed to lookup refractive index for momentum {} eV", mom);
        }

        // add each `irt_photon` to the radiator history
        // - unless cheating, we don't know which photon goes with which
        // radiator, thus we add them all to each radiator; the radiators'
        // photons are mixed in `ChargedParticle::PIDReconstruction`
        irt_rad_history->AddOpticalPhoton(irt_photon);
        /* FIXME: this considers ALL of the `irt_photon`s... we can limit this
         * once we add the ability to get a fiducial volume for each track, i.e.,
         * a region of sensors where we expect to see this `irt_particle`'s
         * Cherenkov photons; this should also combat sensor noise
         */
      } // end `in_hit_assocs` loop

    } // end radiator loop



    // particle identification +++++++++++++++++++++++++++++++++++++++++++++++++++++

    // define a mass hypothesis for each particle we want to check
    m_log->trace("{:+^70}"," PARTICLE IDENTIFICATION ");
    CherenkovPID irt_pid;
    std::unordered_map<int,MassHypothesis*> pdg_to_hyp; // `pdg` -> hypothesis
    for(auto [pdg,mass] : m_pdg_mass) {
      irt_pid.AddMassHypothesis(mass);
      pdg_to_hyp.insert({ pdg, irt_pid.GetHypothesis(irt_pid.GetHypothesesCount()-1) });
    }

    // run IRT PID
    irt_particle->PIDReconstruction(irt_pid);
    m_log->trace("{:-^70}"," IRT RESULTS ");

    // loop over radiators
    for(auto [rad_name,irt_rad] : m_pid_radiators) {
      m_log->trace("-> {} Radiator (ID={}):", rad_name, irt_rad->m_ID);

      // Cherenkov angle (theta) estimate
      unsigned npe        = 0;
      double   rindex_ave = 0.0;
      double   energy_ave = 0.0;
      std::vector<std::pair<double,double>> phot_theta_phi;

      // loop over this radiator's photons, and decide which to include in the theta estimate
      auto *irt_rad_history = irt_particle->FindRadiatorHistory(irt_rad);
      if(irt_rad_history==nullptr) {
        m_log->trace("  No radiator history; skip");
        continue;
      }
      m_log->trace("  Photoelectrons:");
      for(auto *irt_photon : irt_rad_history->Photons()) {

        // check whether this photon was selected by at least one mass hypothesis
        bool photon_selected = false;
        for(auto irt_photon_sel : irt_photon->_m_Selected) {
          if(irt_photon_sel.second == irt_rad) {
            photon_selected = true;
            break;
          }
        }
        if(!photon_selected) continue;

        // trace logging
        Tools::PrintTVector3(
            m_log,
            fmt::format("- sensor_id={:#X}: hit",irt_photon->GetVolumeCopy()),
            irt_photon->GetDetectionPosition()
            );
        Tools::PrintTVector3(m_log, "photon vertex", irt_photon->GetVertexPosition());

        // get this photon's theta and phi estimates
        auto phot_theta = irt_photon->_m_PDF[irt_rad].GetAverage();
        auto phot_phi   = irt_photon->m_Phi[irt_rad];

        // add to the total
        npe++;
        phot_theta_phi.emplace_back( phot_theta, phot_phi );
        if(m_cfg.cheatPhotonVertex) {
          rindex_ave += irt_photon->GetVertexRefractiveIndex();
          energy_ave += irt_photon->GetVertexMomentum().Mag();
        }

      } // end loop over this radiator's photons

      // compute averages
      if(npe>0) {
        rindex_ave /= npe;
        energy_ave /= npe;
      }

      // fill photon info
      auto out_cherenkov_pid = out_cherenkov_pids.at(rad_name)->create();
      out_cherenkov_pid.setNpe(static_cast<decltype(edm4eic::CherenkovParticleIDData::npe)>(npe));
      out_cherenkov_pid.setRefractiveIndex(static_cast<decltype(edm4eic::CherenkovParticleIDData::refractiveIndex)>(rindex_ave));
      out_cherenkov_pid.setPhotonEnergy(static_cast<decltype(edm4eic::CherenkovParticleIDData::photonEnergy)>(energy_ave));
      for(auto [phot_theta,phot_phi] : phot_theta_phi)
        out_cherenkov_pid.addToThetaPhiPhotons(edm4hep::Vector2f{
            static_cast<float>(phot_theta),
            static_cast<float>(phot_phi)
            });

      // relate mass hypotheses
      for(auto [pdg,mass] : m_pdg_mass) {

        // get hypothesis results
        auto *irt_hypothesis = pdg_to_hyp.at(pdg);
        auto hyp_weight     = irt_hypothesis->GetWeight(irt_rad);
        auto hyp_npe        = irt_hypothesis->GetNpe(irt_rad);

        // fill `ParticleID` output collection
        edm4eic::CherenkovParticleIDHypothesis out_hypothesis;
        out_hypothesis.PDG    = static_cast<decltype(edm4eic::CherenkovParticleIDHypothesis::PDG)>(pdg);
        out_hypothesis.weight = static_cast<decltype(edm4eic::CherenkovParticleIDHypothesis::weight)>(hyp_weight);
        out_hypothesis.npe    = static_cast<decltype(edm4eic::CherenkovParticleIDHypothesis::npe)>(hyp_npe);

        // relate
        out_cherenkov_pid.addToHypotheses(out_hypothesis);

      } // end hypothesis loop

      // logging
      Tools::PrintCherenkovEstimate(m_log, out_cherenkov_pid);

      // relate charged particle projection
      auto charged_particle_list_it = in_charged_particles.find("Merged");
      if(charged_particle_list_it != in_charged_particles.end()) {
        const auto *charged_particle_list = charged_particle_list_it->second;
        auto charged_particle      = charged_particle_list->at(i_charged_particle);
        out_cherenkov_pid.setChargedParticle(charged_particle);
      }
      else
        m_log->error("Cannot find radiator 'Merged' in `in_charged_particles`");

      // relate hit associations
      for(const auto& hit_assoc : *in_hit_assocs)
        out_cherenkov_pid.addToRawHitAssociations(hit_assoc);

    } // end radiator loop

    /* NOTE: `unique_ptr irt_particle` goes out of scope and will now be destroyed, and along with it:
     * - raw pointer `irt_rad_history` for each radiator
     * - all `irt_photon` raw pointers
     */

  } // end `in_charged_particles` loop
}

} // namespace eicrecon
