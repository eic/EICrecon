// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 EICrecon authors

#include "PandoraInputMapper.h"

#include <cstdint>
#include <edm4eic/TrackPoint.h>
#include <edm4eic/TrackSegment.h>
#include <edm4hep/utils/vector_utils.h>
#include <Pandora/PandoraInputTypes.h>

namespace eicrecon {

void PandoraInputMapper::addCaloHits(pandora::Pandora& pandora,
                                     const edm4eic::CalorimeterHitCollection& hits,
                                     pandora::HitType hitType, pandora::HitRegion hitRegion,
                                     float mipEquivScale) {
  for (const auto& hit : hits) {
    const auto& pos = hit.getPosition(); // mm

    PandoraApi::CaloHit::Parameters params;
    params.m_positionVector = pandora::CartesianVector(pos.x, pos.y, pos.z);
    // Expected direction: unit vector pointing outward from IP
    const float rxy = std::sqrt(pos.x * pos.x + pos.y * pos.y);
    const float r   = std::sqrt(pos.x * pos.x + pos.y * pos.y + pos.z * pos.z);
    if (r > 0.f) {
      params.m_expectedDirection = pandora::CartesianVector(pos.x / r, pos.y / r, pos.z / r);
    } else {
      params.m_expectedDirection = pandora::CartesianVector(0.f, 0.f, 1.f);
    }
    // Cell normal: radial unit vector in transverse plane (barrel) or along z (endcap)
    if (hitRegion == pandora::BARREL && rxy > 0.f) {
      params.m_cellNormalVector = pandora::CartesianVector(pos.x / rxy, pos.y / rxy, 0.f);
    } else {
      params.m_cellNormalVector = pandora::CartesianVector(0.f, 0.f, pos.z > 0 ? 1.f : -1.f);
    }

    // Cell geometry: use rectangular approximation
    params.m_cellGeometry = pandora::RECTANGULAR;
    // Use hit dimensions if available; fall back to zeros (Pandora will use defaults)
    const auto& dim        = hit.getDimension(); // mm
    params.m_cellSize0     = dim.x;
    params.m_cellSize1     = dim.y;
    params.m_cellThickness = dim.z;

    // Material budget: not available per-hit without DDRec; set to zero.
    // The geometry registration provides cumulative values per layer.
    params.m_nCellRadiationLengths   = 0.f;
    params.m_nCellInteractionLengths = 0.f;

    params.m_time                  = hit.getTime();   // ns
    params.m_inputEnergy           = hit.getEnergy(); // GeV
    params.m_mipEquivalentEnergy   = hit.getEnergy() * mipEquivScale;
    params.m_electromagneticEnergy = (hitType == pandora::ECAL) ? hit.getEnergy() : 0.f;
    params.m_hadronicEnergy        = (hitType == pandora::HCAL) ? hit.getEnergy() : 0.f;

    params.m_isDigital              = false;
    params.m_hitType                = hitType;
    params.m_hitRegion              = hitRegion;
    params.m_layer                  = static_cast<unsigned int>(hit.getLayer());
    params.m_isInOuterSamplingLayer = false;
    // Use the cell ID as a stable parent address proxy.
    params.m_pParentAddress =
        reinterpret_cast<const void*>(static_cast<uintptr_t>(hit.getCellID()));

    PANDORA_THROW_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=,
                            PandoraApi::CaloHit::Create(pandora, params));
  }
}

void PandoraInputMapper::addTracks(pandora::Pandora& pandora,
                                   const edm4eic::TrackSegmentCollection& segments) {
  for (const auto& segment : segments) {
    const auto points = segment.getPoints();
    if (points.size() < 1) {
      continue;
    }

    const auto& caloPoint = points[points.size() - 1]; // projection at calo face

    const auto& p_calo = caloPoint.momentum; // GeV
    const auto& x_calo = caloPoint.position; // mm

    const auto& track = segment.getTrack();

    // Perigee position and momentum
    const auto& perigeeMom = track.getMomentum(); // GeV
    const auto& perigeePos = track.getPosition(); // mm

    PandoraApi::Track::Parameters params;
    // d0 and z0: transverse and longitudinal impact parameters at perigee
    params.m_d0 = std::sqrt(perigeePos.x * perigeePos.x + perigeePos.y * perigeePos.y); // mm
    params.m_z0 = perigeePos.z;                                                         // mm
    params.m_particleId = track.getPdg();
    params.m_charge     = static_cast<int>(track.getCharge());
    // Mass: use pion mass as default if PDG is unset (0) or explicitly pion
    const int pdg = track.getPdg();
    params.m_mass = (std::abs(pdg) == 211 || pdg == 0) ? 0.13957f : 0.f;

    params.m_momentumAtDca = pandora::CartesianVector(perigeeMom.x, perigeeMom.y, perigeeMom.z);

    params.m_trackStateAtStart = pandora::TrackState(perigeePos.x, perigeePos.y, perigeePos.z,
                                                     perigeeMom.x, perigeeMom.y, perigeeMom.z);
    params.m_trackStateAtEnd =
        pandora::TrackState(x_calo.x, x_calo.y, x_calo.z, p_calo.x, p_calo.y, p_calo.z);
    params.m_trackStateAtCalorimeter =
        pandora::TrackState(x_calo.x, x_calo.y, x_calo.z, p_calo.x, p_calo.y, p_calo.z);
    params.m_timeAtCalorimeter = caloPoint.time; // ns

    params.m_reachesCalorimeter = true;
    // Determine endcap projection: if the calo projection is near |z| > rxy it's endcap
    const float rxy_calo           = std::sqrt(x_calo.x * x_calo.x + x_calo.y * x_calo.y);
    params.m_isProjectedToEndCap   = (std::abs(x_calo.z) > rxy_calo);
    params.m_canFormPfo            = true;
    params.m_canFormClusterlessPfo = false;
    // Use the track object ID index as a stable parent address proxy.
    params.m_pParentAddress =
        reinterpret_cast<const void*>(static_cast<uintptr_t>(segment.getObjectID().index));

    PANDORA_THROW_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=,
                            PandoraApi::Track::Create(pandora, params));
  }
}

} // namespace eicrecon
