// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 EICrecon authors

#include "PandoraPFA.h"

#include <Api/PandoraApi.h>
#include <Pandora/PandoraEnumeratedTypes.h>

#include "PandoraGeometryMapper.h"
#include "PandoraInputMapper.h"
#include "PandoraOutputMapper.h"

// Include PandoraPFA algorithm factories if available
#ifdef PANDORA_MONITORING
#include <ArborContentMonitoring/PandoraMonitoringApi.h>
#endif

// Try to include PandoraPFA standard algorithms
#if __has_include(<PandoraPFANew/PandoraPFANew.h>)
#include <PandoraPFANew/PandoraPFANew.h>
#endif

namespace eicrecon {

void PandoraPFA::init() {
  m_detector = algorithms::GeoSvc::instance().detector();

  // Create the Pandora instance
  m_pandora = std::make_unique<pandora::Pandora>("EICPandora");

  // Register detector geometry with Pandora
  if (m_detector) {
    PandoraGeometryMapper::registerGeometry(*m_pandora, *m_detector);
  } else {
    warning("DD4hep detector not available; Pandora geometry not registered.");
  }

  // Load algorithm settings from XML
  if (!m_cfg.pandoraSettingsFile.empty()) {
    const pandora::StatusCode sc = PandoraApi::ReadSettings(*m_pandora, m_cfg.pandoraSettingsFile);
    if (sc != pandora::STATUS_CODE_SUCCESS) {
      warning("Could not read Pandora settings file '{}'; running with no algorithms loaded.",
              m_cfg.pandoraSettingsFile);
    }
  }

// Register PandoraPFA algorithm factories if PandoraPFA is available
// This enables the standard PandoraPFA algorithm suite
#ifdef PANDORA_PFARECONSTRUCTION_H
  try {
    pandora::PANDORA_RETURN_RESULT_IF(
        pandora::STATUS_CODE_SUCCESS, !=,
        pandora::PandoraPFANew::RegisterAlgorithmFactories(*m_pandora));
    info("Registered PandoraPFA standard algorithms");
  } catch (const std::exception& e) {
    warning("Could not register PandoraPFA algorithms: {}", e.what());
  }
#endif
}

void PandoraPFA::process(const PandoraPFA::Input& input, const PandoraPFA::Output& output) const {
  const auto [ecalBarrelHits, ecalEndcapHits, hcalBarrelHits, hcalEndcapHits, trackSegments] =
      input;
  auto [outputParticles] = output;

  // Feed calorimeter hits
  PandoraInputMapper::addCaloHits(*m_pandora, *ecalBarrelHits, pandora::ECAL, pandora::BARREL,
                                  m_cfg.ecalMipEquivalentEnergy);
  PandoraInputMapper::addCaloHits(*m_pandora, *ecalEndcapHits, pandora::ECAL, pandora::ENDCAP,
                                  m_cfg.ecalMipEquivalentEnergy);
  PandoraInputMapper::addCaloHits(*m_pandora, *hcalBarrelHits, pandora::HCAL, pandora::BARREL,
                                  m_cfg.hcalMipEquivalentEnergy);
  PandoraInputMapper::addCaloHits(*m_pandora, *hcalEndcapHits, pandora::HCAL, pandora::ENDCAP,
                                  m_cfg.hcalMipEquivalentEnergy);

  // Feed track projections
  PandoraInputMapper::addTracks(*m_pandora, *trackSegments);

  // Run the PFA
  const pandora::StatusCode sc = PandoraApi::ProcessEvent(*m_pandora);
  if (sc != pandora::STATUS_CODE_SUCCESS) {
    error("PandoraApi::ProcessEvent returned error code {}", static_cast<int>(sc));
    return;
  }

  // Extract PFOs into output collection
  PandoraOutputMapper::retrievePFOs(*m_pandora, *outputParticles);

  // Reset Pandora for the next event
  PANDORA_THROW_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraApi::Reset(*m_pandora));

  debug("PandoraPFA produced {} reconstructed particles", outputParticles->size());
}

} // namespace eicrecon
