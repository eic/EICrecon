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
 *        if candidate has Hcal clusters
 *    2. Calculate track and total calorimeter energy as needed, and
 *       take weighted average
 *    3. Using mass and energy, calculate remaining kinematics
 */
void ParticleConverter::process(const Input& input, const Output& output) const {
  const auto [inParticles, inVertices] = input;
  auto [outParticles]                  = output;

  if (inParticles->size() == 0) {
    debug("No particle candidates in the input.");

    return;
  } else {
    debug("There are {} particle candidates in the input.", inParticles->size());
  }

  // NOTE1: in the no-track case, the momentum direction is estimated with the help of the primary vertex.
  // NOTE2: it is asummed the production vertex is the PV
  edm4hep::Vector3f primaryVertex(0.0, 0.0, 0.0);
  if (inVertices->size() > 0) {
    primaryVertex.x = (*inVertices)[0].getPosition().x;
    primaryVertex.y = (*inVertices)[0].getPosition().y;
    primaryVertex.z = (*inVertices)[0].getPosition().z;
  } else {
    debug("No primary vertex found. Set PV to (0,0,0).");
  }

  for (const auto particle : *inParticles) {
    bool hasTrack = false;
    bool hasHcal  = false;
    bool hasEcal  = false;

    double prelimPID = 0;

    double trackEnergy      = 0;
    double trackMass        = 0;
    double trackMomentumMag = 0;

    double caloEnergy = 0;
    double ecalEnergy = 0;
    double hcalEnergy = 0;

    double reconstructedEnergy = 0;

    edm4hep::Vector3f ecalClusterPosition(0.0, 0.0, 0.0);
    edm4hep::Vector3f hcalClusterPosition(0.0, 0.0, 0.0);

    edm4hep::Vector3f trackMomentum(0.0, 0.0, 0.0);

    edm4hep::Vector3f reconstructedMomentum(0.0, 0.0, 0.0);

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

      trackMomentum = track.getMomentum();
      prelimPID     = track.getPdg();
      trackMass     = m_particleSvc.particle(prelimPID).mass;

      trackMomentumMag =
          std::sqrt(std::pow(trackMomentum.x, 2) + std::pow(trackMomentum.y, 2) +
                    std::pow(trackMomentum.z, 2));

      trackEnergy = std::sqrt(std::pow(trackMomentumMag, 2) + std::pow(trackMass, 2));

      reconstructedMomentum.x = trackMomentum.x;
      reconstructedMomentum.y = trackMomentum.y;
      reconstructedMomentum.z = trackMomentum.z;
    }

    // Looking for clusters
    for (auto cluster : particle.getClusters()) {
      if (cluster.getHits().size() <= 0) {
        trace("Cluster {} has no hits", cluster.getObjectID().index);
        continue;
      }

      const auto caloHit = cluster.getHits()[0];
      const auto cellId  = caloHit.getCellID();

      const dd4hep::VolumeManagerContext* context = m_converter->findContext(cellId);

      if (context) {
        const dd4hep::DetElement detectorElement = context->element;
        const dd4hep::DetType type(detectorElement.typeFlag());

        std::string detectorElementType = detectorElement.type();

        // When all testing is done, return to this form of Calo discriminator
        // if (type.is(dd4hep::DetType::CALORIMETER) && type.is(dd4hep::DetType::ELECTROMAGNETIC))
        //   hasEcal = true;

        // if (type.is(dd4hep::DetType::CALORIMETER) && type.is(dd4hep::DetType::HADRONIC))
        //   hasHcal = true;

        std::string ecalString = "Ecal";
        std::string hcalString = "Hcal";

        if (detectorElementType.find(ecalString) != std::string::npos)
          hasEcal = true;

        if (detectorElementType.find(hcalString) != std::string::npos)
          hasHcal = true;
      }

      if (hasEcal) {
        ecalEnergy += cluster.getEnergy();

        ecalClusterPosition.x += cluster.getEnergy() * cluster.getPosition().x;
        ecalClusterPosition.y += cluster.getEnergy() * cluster.getPosition().y;
        ecalClusterPosition.z += cluster.getEnergy() * cluster.getPosition().z;
      }

      if (hasHcal) {
        hcalEnergy += cluster.getEnergy();

        hcalClusterPosition.x += cluster.getEnergy() * cluster.getPosition().x;
        hcalClusterPosition.y += cluster.getEnergy() * cluster.getPosition().y;
        hcalClusterPosition.z += cluster.getEnergy() * cluster.getPosition().z;
      }
    }

    // Energy weighted cluster position to determine momentum direction of non-track
    if (hasEcal) {
      ecalClusterPosition.x /= ecalEnergy;
      ecalClusterPosition.y /= ecalEnergy;
      ecalClusterPosition.z /= ecalEnergy;
    }

    if (hasHcal) {
      hcalClusterPosition.x /= hcalEnergy;
      hcalClusterPosition.y /= hcalEnergy;
      hcalClusterPosition.z /= hcalEnergy;
    }

    if (hasEcal && !hasHcal && !hasTrack)
      prelimPID = 22; //photon
    if (!hasEcal && hasHcal && !hasTrack)
      prelimPID = 2112; // neutron

    // Step 2 : Calculate calo energy (PRELIMINARY IMPLEMENTATION)
    // TODO: the case where there is only energy in the Hcal might require a specialized normalization in the future.
    if (hasEcal) {
      caloEnergy += ecalEnergy;

      if (hasHcal) {
        caloEnergy += m_cfg.caloHadronScale * hcalEnergy;
        caloEnergy *= m_cfg.caloEnergyNorm;
      }
    } else if (hasHcal) {
      caloEnergy += hcalEnergy;
    }

    // Step 3 : Calculate avge energy (PRELIMINARY IMPLEMENTATION)
    // Note: USING ECAL RESOLUTION AS PLACEHOLDER!
    double weightTrackingResolution = 1. / std::pow(m_cfg.trackingResolution, 2);
    double weightCaloResolution =
        1. / std::pow(m_cfg.caloResolution, 2);

    double normalization = 0;

    if (trackEnergy > 0 && caloEnergy > 0)
      normalization = weightTrackingResolution + weightCaloResolution;
    else if (trackEnergy > 0 && caloEnergy == 0)
      normalization = weightTrackingResolution;
    else if (trackEnergy == 0 && caloEnergy > 0)
      normalization = weightCaloResolution;

    if (!hasTrack) {
      reconstructedEnergy = caloEnergy;
    } else if (hasEcal || hasHcal) {
      reconstructedEnergy =
          (weightTrackingResolution * trackEnergy + weightCaloResolution * caloEnergy) /
          normalization;
    } else {
      reconstructedEnergy = trackEnergy;
    }

    // Step 5 (for neutrals): estimate the momentum direction
    edm4hep::Vector3f neutralParticleDirection(0.0, 0.0, 0.0);

    if (hasEcal) {
      neutralParticleDirection.x = ecalClusterPosition.x - primaryVertex.x;
      neutralParticleDirection.y = ecalClusterPosition.y - primaryVertex.y;
      neutralParticleDirection.z = ecalClusterPosition.z - primaryVertex.z;
    }

    if (hasHcal) {
      neutralParticleDirection.x = hcalClusterPosition.x - primaryVertex.x;
      neutralParticleDirection.y = hcalClusterPosition.y - primaryVertex.y;
      neutralParticleDirection.z = hcalClusterPosition.z - primaryVertex.z;
    }

    if (neutralParticleDirection.x != 0 && neutralParticleDirection.y != 0 &&
        neutralParticleDirection.z != 0) {
      double magNeutralDirection = std::sqrt(std::pow(neutralParticleDirection.x, 2) +
                                             std::pow(neutralParticleDirection.y, 2) +
                                             std::pow(neutralParticleDirection.z, 2));

      neutralParticleDirection.x /= magNeutralDirection;
      neutralParticleDirection.y /= magNeutralDirection;
      neutralParticleDirection.z /= magNeutralDirection;
    }

    if ((hasEcal || hasHcal) && !hasTrack) {
      double neutralsMomentumMag = std::sqrt(std::pow(reconstructedEnergy, 2) -
                                             std::pow(m_particleSvc.particle(prelimPID).mass, 2));

      reconstructedMomentum.x = neutralParticleDirection.x * neutralsMomentumMag;
      reconstructedMomentum.y = neutralParticleDirection.y * neutralsMomentumMag;
      reconstructedMomentum.z = neutralParticleDirection.z * neutralsMomentumMag;
    }

    // Step 6: write on the out output collection
    double reconstructedMomentumMag = std::sqrt(std::pow(reconstructedMomentum.x, 2) +
                                                std::pow(reconstructedMomentum.y, 2) +
                                                std::pow(reconstructedMomentum.z, 2));

    double reconstructedMass =
        std::sqrt(std::pow(reconstructedEnergy, 2) - std::pow(reconstructedMomentumMag, 2));

    double reconstructedPID = prelimPID;

    edm4eic::MutableReconstructedParticle outRecoParticle = particle.clone();

    if (std::isnan(reconstructedMass)) {
      debug("Mass of this particle was NaN -> M2 = {}",
            std::pow(reconstructedEnergy, 2) - std::pow(reconstructedMomentumMag, 2));
    }

    if (hasTrack && trackMomentum.x == 0) {
      debug("Track with null momentum. PID = {}", prelimPID);
      debug("Ecal  = {}", hasEcal);
      debug("Hcal  = {}", hasHcal);
      debug("Preco = ({}, {}, {})", reconstructedMomentum.x, reconstructedMomentum.y,
            reconstructedMomentum.z);
      debug("Ptrck = ({}, {}, {})", trackMomentum.x, trackMomentum.y,
            trackMomentum.z);
    }

    outRecoParticle.setMomentum(edm4hep::Vector3f(reconstructedMomentum.x,
                                                  reconstructedMomentum.y,
                                                  reconstructedMomentum.z));
    outRecoParticle.setEnergy(reconstructedEnergy);
    outRecoParticle.setCharge(m_particleSvc.particle(reconstructedPID).charge);
    outRecoParticle.setMass(reconstructedMass);
    outRecoParticle.setPDG(reconstructedPID);

    outParticles->push_back(outRecoParticle);
  }
};
} // namespace eicrecon
