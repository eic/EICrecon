// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Esteban Molina, Derek Anderson

#include <DD4hep/Detector.h>
#include <DD4hep/DetType.h>
#include <DDRec/CellIDPositionConverter.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/ReconstructedParticle.h>
#include <edm4eic/MutableReconstructedParticle.h>

#include "ParticleConverter.h"

namespace eicrecon {
// ----------------------------------------------------------------------------
//! Process inputs
// ----------------------------------------------------------------------------
/*! Convert a particle candidate into a reconstructed particle
 *  with well defined kinematics and PID via the following
 *  algorithm:
 *    1.  Assign rough PDG code based on presence of tracks and clusters
 *      - If candidate has a track, use its associated PID
 *      - Otherwise label it as a photon or neutron based on
 *        if candidate has HCal clusters
 *    2. Calculate track and total calorimeter energy as needed, and
 *       take weighted average
 *    3. Using mass and energy, calculate remaining kinematics
 */
void ParticleConverter::process(const Input& input, const Output& output) const {
  const auto [in_particles] = input;
  auto [out_particles]      = output;

  if (in_particles->size() == 0) {
    debug("No particle candidates in the input.");

    return;
  } else {
    debug("There are {} particle candidates in the input.", in_particles->size());
  }

  for (const auto particle : *in_particles) {
    bool hasTrack = false;
    bool hasHCal  = false;
    bool hasECal  = false;

    double prelim_pid = 0;

    double track_energy       = 0;
    double track_mass         = 0;
    double track_momentum_mag = 0;

    double calo_energy = 0;
    double ecal_energy = 0;
    double hcal_energy = 0;

    double avge_energy = 0;

    edm4hep::Vector3f track_momentum_vector;

    // Step 1 : Assign preliminary PID

    // Looking for tracks
    std::size_t nTracks = particle.getTracks().size();
    edm4eic::Track track;

    if (nTracks > 0) {
      if (nTracks > 1) {
        trace("Particle candidate {} has {} related tracks. Using first.",
              particle.getObjectID().index, nTracks);
      }

      track = particle.getTracks()[0];
    }

    if (track.getChi2() > 0) {
      hasTrack = true;

      track_momentum_vector = track.getMomentum();
      prelim_pid            = track.getPdg();
      track_mass            = m_particleSvc.particle(prelim_pid).mass;
      
      track_momentum_mag =
          std::sqrt(std::pow(track_momentum_vector.x, 2) + std::pow(track_momentum_vector.y, 2) +
                    std::pow(track_momentum_vector.z, 2));

      track_energy = std::sqrt(std::pow(track_momentum_mag, 2) + std::pow(track_mass, 2));
    }

    // Looking for clusters
    for (auto cluster : particle.getClusters()) {
      if (cluster.getHits().size() <= 0) {
        trace("Cluster {} has no hits", cluster.getObjectID().index);
        continue;
      }

      const auto calo_hit = cluster.getHits()[0];

      /* check type */
      const auto cell_id = calo_hit.getCellID();

      const dd4hep::VolumeManagerContext* context = m_converter->findContext(cell_id);

      if (context) {
        const dd4hep::DetElement det_element = context->element;
        const dd4hep::DetType type(det_element.typeFlag());

        std::string det_element_type = det_element.type();

        // Note: will probably change
        if (type.is(dd4hep::DetType::CALORIMETER) && type.is(dd4hep::DetType::ELECTROMAGNETIC))
          hasECal = true;

        if (type.is(dd4hep::DetType::CALORIMETER) && type.is(dd4hep::DetType::HADRONIC))
          hasHCal = true;
      }

      if (hasECal)
        ecal_energy += cluster.getEnergy();

      if (hasHCal)
        hcal_energy += cluster.getEnergy();
    }

    if (hasECal && !hasHCal && !hasTrack)
      prelim_pid = 22; //photon
    if (!hasECal && hasHCal && !hasTrack)
      prelim_pid = 2112; // neutron

    // Step 2 : Calculate calo energy (PRELIMINARY IMPLEMENTATION)
    // TODO: the case where there is only energy in the HCal might require a specialized normalization in the future.
    if (hasECal) {
      calo_energy += ecal_energy;

      if (hasHCal) {
        calo_energy += m_cfg.caloHadronScale * hcal_energy;
        calo_energy *= m_cfg.caloEnergyNorm;
      }
    } else if (hasHCal) {
      calo_energy += hcal_energy;
    }

    // Step 3 : Calculate avge energy (PRELIMINARY IMPLEMENTATION)
    double weight_trackingResolution = 1. / std::pow(m_cfg.trackingResolution, 2);
    double weight_calo_resolution =
        1. / std::pow(m_cfg.caloResolution, 2); // USING ECAL RESOLUTION AS PLACEHOLDER!

    double normalization = 0;

    if (track_energy > 0 && calo_energy > 0)
      normalization = weight_trackingResolution + weight_calo_resolution;
    else if (track_energy > 0 && calo_energy == 0)
      normalization = weight_trackingResolution;
    else if (track_energy == 0 && calo_energy > 0)
      normalization = weight_calo_resolution;

    avge_energy =
        (weight_trackingResolution * track_energy + weight_calo_resolution * calo_energy) /
        normalization;

    // Step 4 : Store information on a mutable collection
    double mass_calculated = std::sqrt(std::pow(avge_energy, 2) - std::pow(track_momentum_mag, 2));

    edm4eic::MutableReconstructedParticle out_reco_particle = particle.clone();

    out_reco_particle.setEnergy(avge_energy);
    out_reco_particle.setMomentum(edm4hep::Vector3f(
        track_momentum_vector.x, track_momentum_vector.y, track_momentum_vector.z));
    out_reco_particle.setReferencePoint(particle.getReferencePoint());

    out_reco_particle.setCharge(particle.getCharge());
    out_reco_particle.setMass(mass_calculated);
    out_reco_particle.setGoodnessOfPID(particle.getGoodnessOfPID());
    out_reco_particle.setCovMatrix(particle.getCovMatrix());
    out_reco_particle.setPDG(prelim_pid);

    out_reco_particle.setStartVertex(particle.getStartVertex());
    out_reco_particle.setParticleIDUsed(particle.getParticleIDUsed());

    out_particles->push_back(out_reco_particle);
  }
};
} // namespace eicrecon
