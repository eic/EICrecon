// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Esteban Molina, Derek Anderson

#include <DD4hep/Detector.h>
#include <DD4hep/DetType.h>
#include <DDRec/CellIDPositionConverter.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/ReconstructedParticle.h>
#include <edm4eic/VertexCollection.h>
#include <edm4eic/Vertex.h>
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
  const auto [in_particles, in_vertices] = input;
  auto [out_particles]                   = output;

  if (in_particles->size() == 0) {
    debug("No particle candidates in the input.");

    return;
  } else {
    debug("There are {} particle candidates in the input.", in_particles->size());
  }

  // NOTE1: in the no-track case, the momentum direction is estimated with the help of the primary vertex.
  // NOTE2: it is asummed the production vertex is the PV
  edm4hep::Vector3f primary_vertex(0.0, 0.0, 0.0);
  if (in_vertices->size() > 0) {
    primary_vertex.x = (*in_vertices)[0].getPosition().x;
    primary_vertex.y = (*in_vertices)[0].getPosition().y;
    primary_vertex.z = (*in_vertices)[0].getPosition().z;
  } else {
    debug("No primary vertex found. Set PV to (0,0,0).");
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

    double reconstructed_energy = 0;

    edm4hep::Vector3f ecal_cluster_position(0.0, 0.0, 0.0);
    edm4hep::Vector3f hcal_cluster_position(0.0, 0.0, 0.0);

    edm4hep::Vector3f track_momentum_vector(0.0, 0.0, 0.0);

    edm4hep::Vector3f reconstructed_momentum_vector(0.0, 0.0, 0.0);

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

      reconstructed_momentum_vector.x = track_momentum_vector.x;
      reconstructed_momentum_vector.y = track_momentum_vector.y;
      reconstructed_momentum_vector.z = track_momentum_vector.z;
    }

    // Looking for clusters
    for (auto cluster : particle.getClusters()) {
      if (cluster.getHits().size() <= 0) {
        trace("Cluster {} has no hits", cluster.getObjectID().index);
        continue;
      }

      const auto calo_hit = cluster.getHits()[0];

      const auto cell_id = calo_hit.getCellID();

      const dd4hep::VolumeManagerContext* context = m_converter->findContext(cell_id);

      if (context) {
        const dd4hep::DetElement det_element = context->element;
        const dd4hep::DetType type(det_element.typeFlag());

        std::string det_element_type = det_element.type();

        // When all testing is done, return to this form of Calo discriminator
        // if (type.is(dd4hep::DetType::CALORIMETER) && type.is(dd4hep::DetType::ELECTROMAGNETIC))
        //   hasECal = true;

        // if (type.is(dd4hep::DetType::CALORIMETER) && type.is(dd4hep::DetType::HADRONIC))
        //   hasHCal = true;

        std::string ecal_string = "Ecal";
        std::string hcal_string = "Hcal";

        if (det_element_type.find(ecal_string) != std::string::npos)
          hasECal = true;

        if (det_element_type.find(hcal_string) != std::string::npos)
          hasHCal = true;
      }

      if (hasECal) {
        ecal_energy += cluster.getEnergy();

        ecal_cluster_position.x += cluster.getEnergy() * cluster.getPosition().x;
        ecal_cluster_position.y += cluster.getEnergy() * cluster.getPosition().y;
        ecal_cluster_position.z += cluster.getEnergy() * cluster.getPosition().z;
      }

      if (hasHCal) {
        hcal_energy += cluster.getEnergy();

        hcal_cluster_position.x += cluster.getEnergy() * cluster.getPosition().x;
        hcal_cluster_position.y += cluster.getEnergy() * cluster.getPosition().y;
        hcal_cluster_position.z += cluster.getEnergy() * cluster.getPosition().z;
      }
    }

    // Energy weighted cluster position to determine momentum direction of non-track
    if (hasECal) {
      ecal_cluster_position.x /= ecal_energy;
      ecal_cluster_position.y /= ecal_energy;
      ecal_cluster_position.z /= ecal_energy;
    }

    if (hasHCal) {
      hcal_cluster_position.x /= hcal_energy;
      hcal_cluster_position.y /= hcal_energy;
      hcal_cluster_position.z /= hcal_energy;
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
    double weight_caloResolution =
        1. / std::pow(m_cfg.caloResolution, 2); // USING ECAL RESOLUTION AS PLACEHOLDER!

    double normalization = 0;

    if (track_energy > 0 && calo_energy > 0)
      normalization = weight_trackingResolution + weight_caloResolution;
    else if (track_energy > 0 && calo_energy == 0)
      normalization = weight_trackingResolution;
    else if (track_energy == 0 && calo_energy > 0)
      normalization = weight_caloResolution;

    if (!hasTrack) {
      reconstructed_energy = calo_energy;
    } else if (hasECal || hasHCal) {
      reconstructed_energy =
          (weight_trackingResolution * track_energy + weight_caloResolution * calo_energy) /
          normalization;
    } else {
      reconstructed_energy = track_energy;
    }

    // Step 5 (for neutrals): estimate the momentum direction
    edm4hep::Vector3f neutrals_direction(0.0, 0.0, 0.0);

    if (hasECal) {
      debug("ECAL!!!!!!!!!!");

      neutrals_direction.x = ecal_cluster_position.x - primary_vertex.x;
      neutrals_direction.y = ecal_cluster_position.y - primary_vertex.y;
      neutrals_direction.z = ecal_cluster_position.z - primary_vertex.z;
    }

    if (hasHCal) {
      neutrals_direction.x = hcal_cluster_position.x - primary_vertex.x;
      neutrals_direction.y = hcal_cluster_position.y - primary_vertex.y;
      neutrals_direction.z = hcal_cluster_position.z - primary_vertex.z;
    }

    if (neutrals_direction.x != 0 && neutrals_direction.y != 0 && neutrals_direction.z != 0) {
      double mag_neutrals_direction =
          std::sqrt(std::pow(neutrals_direction.x, 2) + std::pow(neutrals_direction.y, 2) +
                    std::pow(neutrals_direction.z, 2));

      neutrals_direction.x /= mag_neutrals_direction;
      neutrals_direction.y /= mag_neutrals_direction;
      neutrals_direction.z /= mag_neutrals_direction;
    }

    if ((hasECal || hasHCal) && !hasTrack) {
      double neutrals_momentum_mag = std::sqrt(
          std::pow(reconstructed_energy, 2) - std::pow(m_particleSvc.particle(prelim_pid).mass, 2));

      reconstructed_momentum_vector.x = neutrals_direction.x * neutrals_momentum_mag;
      reconstructed_momentum_vector.y = neutrals_direction.y * neutrals_momentum_mag;
      reconstructed_momentum_vector.z = neutrals_direction.z * neutrals_momentum_mag;
    }

    // Step 6: write on the out output collection
    double reconstructed_momentum_mag = std::sqrt(std::pow(reconstructed_momentum_vector.x, 2) +
                                                  std::pow(reconstructed_momentum_vector.y, 2) +
                                                  std::pow(reconstructed_momentum_vector.z, 2));

    double reconstructed_mass =
        std::sqrt(std::pow(reconstructed_energy, 2) - std::pow(reconstructed_momentum_mag, 2));

    edm4eic::MutableReconstructedParticle out_reco_particle = particle.clone();

    debug("P = ({}, {}, {})", reconstructed_momentum_vector.x, reconstructed_momentum_vector.y,
          reconstructed_momentum_vector.z);
    debug("M = {}", reconstructed_mass);
    debug("E = {}", reconstructed_energy);

    if (std::isnan(reconstructed_mass)) {
      debug("M2 = {}", std::pow(reconstructed_energy, 2) - std::pow(reconstructed_momentum_mag, 2));
    }

    if (hasTrack && track_momentum_vector.x == 0) {
      debug("Preco = ({}, {}, {})", reconstructed_momentum_vector.x,
            reconstructed_momentum_vector.y, reconstructed_momentum_vector.z);
      debug("Ptrck = ({}, {}, {})", track_momentum_vector.x, track_momentum_vector.y,
            track_momentum_vector.z);
    }

    out_reco_particle.setMomentum(edm4hep::Vector3f(reconstructed_momentum_vector.x,
                                                    reconstructed_momentum_vector.y,
                                                    reconstructed_momentum_vector.z));
    out_reco_particle.setEnergy(reconstructed_energy);
    out_reco_particle.setCharge(m_particleSvc.particle(prelim_pid).charge);
    out_reco_particle.setMass(reconstructed_mass);
    out_reco_particle.setPDG(prelim_pid);

    out_particles->push_back(out_reco_particle);
  }
};
} // namespace eicrecon
