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
    bool isTrack = false;
    bool isHCal  = false;
    bool isECal  = false;

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
    for (auto track : particle.getTracks()) {
      isTrack = true;

      prelim_pid            = particle.getPDG();
      track_mass            = particle.getMass();
      track_momentum_vector = particle.getMomentum();

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
          isECal = true;

        if (type.is(dd4hep::DetType::CALORIMETER) && type.is(dd4hep::DetType::HADRONIC))
          isHCal = true;
      }

      if (isECal)
        ecal_energy += cluster.getEnergy();

      if (isHCal)
        hcal_energy += cluster.getEnergy();
    }

    if (isECal && !isHCal && !isTrack)
      prelim_pid = 22; //photon
    if (!isECal && isHCal && !isTrack)
      prelim_pid = 2112; // neutron

    // Step 2 : Calculate calo energy (PRELIMINARY IMPLEMENTATION)
    if (ecal_energy > 0)
      calo_energy += ecal_energy;
    if (hcal_energy > 0)
      calo_energy += m_cfg.calo_hadron_scale * hcal_energy;

    calo_energy *= m_cfg.calo_energy_norm;

    // Step 3 : Calculate avge energy (PRELIMINARY IMPLEMENTATION)
    double weight_tracking_resolution = 1. / std::pow(m_cfg.tracking_resolution, 2);
    double weight_calo_resolution =
        1. / std::pow(m_cfg.ecal_resolution, 2); // USING ECAL RESOLUTION AS PLACEHOLDER!

    double normalization = 0;

    if (track_energy > 0 && calo_energy > 0)
      normalization = weight_tracking_resolution + weight_calo_resolution;
    else if (track_energy > 0 && calo_energy == 0)
      normalization = weight_tracking_resolution;
    else if (track_energy == 0 && calo_energy > 0)
      normalization = weight_calo_resolution;

    avge_energy = (weight_tracking_resolution * track_energy + weight_calo_resolution * calo_energy) /
                  normalization;

    // Step 4 : Store information on a mutable collection
    double mass_calculated = std::sqrt(std::pow(avge_energy, 2) - std::pow(track_momentum_mag, 2));

    edm4eic::MutableReconstructedParticle out_reco_particle = particle.clone();

    out_reco_particle.setEnergy(avge_energy);
    out_reco_particle.setMomentum(
        edm4hep::Vector3f(track_momentum_vector.x, track_momentum_vector.y, track_momentum_vector.z));
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
