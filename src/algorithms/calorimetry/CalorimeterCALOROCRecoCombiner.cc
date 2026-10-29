// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Chun Yuen Tsang, Minho Kim

#include "CalorimeterCALOROCRecoCombiner.h"

#include <DD4hep/Alignments.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Objects.h>
#include <DD4hep/Readout.h>
#include <DD4hep/VolumeManager.h>
#include <Evaluator/DD4hepUnits.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/CaloHitContribution.h>
#include <edm4hep/MCParticle.h>
#include <podio/ObjectID.h>
#include <podio/RelationRange.h>
#include <podio/detail/Link.h>
#include <podio/detail/LinkCollectionImpl.h>
#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <utility>
#include <vector>

#include "algorithms/calorimetry/CalorimeterCALOROCRecoCombinerConfig.h"

namespace eicrecon {

double CalorimeterCALOROCRecoCombiner::_energyCor(double referencePos, double energy,
                                                  double z) const {
  double length = std::abs(referencePos - z);
  double factor =
      m_cfg.attenuationParameters[0] * std::exp(-length / m_cfg.attenuationParameters[1]) +
      (1 - m_cfg.attenuationParameters[0]) * std::exp(-length / m_cfg.attenuationParameters[2]);
  return energy / factor;
}

void CalorimeterCALOROCRecoCombiner::init() {

  if (m_cfg.attenuationReferencePositionNamePos.empty() ||
      m_cfg.attenuationReferencePositionNameNeg.empty()) {
    error("You MUST provide the name of reference positions for attenuation correction");
  }

  m_reference_z_p = m_detector->constant<double>(m_cfg.attenuationReferencePositionNamePos) *
                    edm4eic::unit::mm / dd4hep::mm;
  m_reference_z_n = m_detector->constant<double>(m_cfg.attenuationReferencePositionNameNeg) *
                    edm4eic::unit::mm / dd4hep::mm;

  if (m_cfg.readout.empty()) {
    error("You MUST provide the name of the readout to decode CellID.");
  }

  // local detector name has higher priority
  if (!m_cfg.localDetElement.empty()) {
    try {
      m_local = m_detector->detector(m_cfg.localDetElement);
    } catch (...) {
      error("failed to load local coordinate system from DetElement {}", m_cfg.localDetElement);
    }
  } else {
    auto id_spec = m_detector->readout(m_cfg.readout).idSpec();
    std::vector<std::pair<std::string, int>> fields;
    for (const auto& f : m_cfg.localDetFields) {
      fields.emplace_back(f, 0);
    }
    local_mask = id_spec.get_mask(fields);
    // use all fields if nothing provided
    if (fields.empty()) {
      local_mask = ~static_cast<decltype(local_mask)>(0);
    }
  }
}

void CalorimeterCALOROCRecoCombiner::process(
    const CalorimeterCALOROCRecoCombiner::Input& input,
    const CalorimeterCALOROCRecoCombiner::Output& output) const {

  const auto [recHitsP, recHitsN, npeHitsP, npeHitsN] = input;
  auto [recohits, rawhits, rawhitsLink, rawhitsAssoc] = output;

  // match the hits from the two sides by cellID
  std::unordered_map<uint64_t, size_t> cellID2RecHitNID, cellID2NpeHitPID, cellID2NpeHitNID;
  for (size_t i = 0; i < recHitsN->size(); ++i) {
    cellID2RecHitNID[recHitsN->at(i).getCellID()] = i;
  }
  for (size_t i = 0; i < npeHitsP->size(); ++i) {
    cellID2NpeHitPID[npeHitsP->at(i).getCellID()] = i;
  }
  for (size_t i = 0; i < npeHitsN->size(); ++i) {
    cellID2NpeHitNID[npeHitsN->at(i).getCellID()] = i;
  }

  for (const auto& recHitP : *recHitsP) {
    const auto cellID = recHitP.getCellID();

    // ignore data point if the hit is not showing up on all the collections
    auto it = cellID2RecHitNID.find(cellID);
    if (it == cellID2RecHitNID.end()) {
      continue;
    }
    const auto& recHitN = recHitsN->at(it->second);
    it                  = cellID2NpeHitPID.find(cellID);
    if (it == cellID2NpeHitPID.end()) {
      continue;
    }
    const auto& npeHitP = npeHitsP->at(it->second);
    it                  = cellID2NpeHitNID.find(cellID);
    if (it == cellID2NpeHitNID.end()) {
      continue;
    }
    const auto& npeHitN = npeHitsN->at(it->second);

    const double tP = recHitP.getTime();
    const double tN = recHitN.getTime();

    // get position of the hit
    double zpos;
    if (m_cfg.useNpeHitPos) {
      zpos = static_cast<double>(npeHitP.getPosition().z);
    } else {
      zpos = (tN - tP) * m_cfg.lightSpeedParameters[0] + m_cfg.lightSpeedParameters[1];
    }
    // bound zpos by sipm location
    zpos = std::max(m_reference_z_n, std::min(m_reference_z_p, zpos));

    const double time = 0.5 * (tP + tN);

    // attenuation correction, then geometric mean of the two ends
    const double corEP = _energyCor(m_reference_z_p, recHitP.getEnergy(), zpos);
    const double corEN = _energyCor(m_reference_z_n, recHitN.getEnergy(), zpos);
    const double corE  = std::sqrt(corEP * corEN);

    // raw hit: keep the P-side amplitude, and the combined time
    edm4hep::MutableRawCalorimeterHit rawhit;
    rawhit.setCellID(cellID);
    rawhit.setAmplitude(recHitP.getRawHit().getAmplitude());
    rawhit.setTimeStamp(time);

    double edep = 0;

    // link to parents
    std::unordered_map<podio::ObjectID, edm4eic::MutableMCRecoCalorimeterHitLink> links_staging;
    std::unordered_map<podio::ObjectID, edm4eic::MutableMCRecoCalorimeterHitAssociation>
        rawassocs_staging;

    for (bool NSide : {true, false}) {
      const auto& npeHit = NSide ? npeHitN : npeHitP;
      for (const auto& contrib : npeHit.getContributions()) {
        if (links_staging.find(contrib.getParticle().getObjectID()) != links_staging.end()) {
          continue;
        }

        edep += contrib.getEnergy();
        edm4eic::MutableMCRecoCalorimeterHitAssociation assoc;
        assoc.setRawHit(rawhit);

        edm4eic::MutableMCRecoCalorimeterHitLink link;
        link.setFrom(rawhit);

        assoc.setSimHit(npeHit);
        assoc.setWeight(contrib.getEnergy());

        link.setTo(npeHit);
        link.setWeight(contrib.getEnergy());

        rawassocs_staging[contrib.getParticle().getObjectID()] = assoc;
        links_staging[contrib.getParticle().getObjectID()]     = link;
      }
    }

    if (edep > 0.0) {
      for (auto& [key, link] : links_staging) {
        auto newLink = rawhitsLink->create();
        newLink.setTo(link.getTo());
        newLink.setFrom(link.getFrom());
        newLink.setWeight(link.getWeight() / edep);
      }
      for (auto& [key, assoc] : rawassocs_staging) {
        assoc.setWeight(assoc.getWeight() / edep);
        rawhitsAssoc->push_back(assoc);
      }
    }

    // x and y come from the single-sided hit, z from the estimate above
    const auto& posP = recHitP.getPosition();
    dd4hep::Position gpos(posP.x * dd4hep::mm, posP.y * dd4hep::mm, zpos * dd4hep::mm);

    dd4hep::Position lpos;
    try {
      dd4hep::DetElement local;
      if (m_cfg.localDetElement.empty()) {
        local = m_detector->volumeManager().lookupDetElement(cellID & local_mask);
      } else {
        local = m_local;
      }
      lpos = local.nominal().worldToLocal(gpos);
    } catch (...) {
      continue;
    }

    const decltype(edm4eic::CalorimeterHitData::position) position(
        gpos.x() / dd4hep::mm, gpos.y() / dd4hep::mm, gpos.z() / dd4hep::mm);
    const decltype(edm4eic::CalorimeterHitData::local) local_position(
        lpos.x() / dd4hep::mm, lpos.y() / dd4hep::mm, lpos.z() / dd4hep::mm);

    auto recohit = recohits->create(cellID, corE, 0, time, 0, position, recHitP.getDimension(),
                                    recHitP.getSector(), recHitP.getLayer(), local_position);
    recohit.setRawHit(rawhit);
    rawhits->push_back(rawhit);
  }
}

} // namespace eicrecon
