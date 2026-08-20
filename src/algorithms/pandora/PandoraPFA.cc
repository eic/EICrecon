// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 EICrecon authors

#include "PandoraPFA.h"

#include <Api/PandoraApi.h>
#include <Pandora/PandoraEnumeratedTypes.h>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <mutex>

// #include "PandoraGeometryMapper.h"  // Temporarily disabled due to DDRec namespace issues
#include "PandoraInputMapper.h"
#include "PandoraOutputMapper.h"
#include "XmlParameterOverride.h"

// Include PandoraPFA algorithm factories if available
#ifdef PANDORA_MONITORING
#include <ArborContentMonitoring/PandoraMonitoringApi.h>
#endif

// Try to include PandoraPFA standard algorithms
#if __has_include(<PandoraPFANew/PandoraPFANew.h>)
#include <PandoraPFANew/PandoraPFANew.h>
#endif

namespace eicrecon {

void PandoraPFA::initializePandora() const {
  m_detector = algorithms::GeoSvc::instance().detector();

  // Create the Pandora instance
  m_pandora = std::make_unique<pandora::Pandora>("EICPandora");

  // Register detector geometry with Pandora
  // TODO: Fix DDRec namespace issues in PandoraGeometryMapper
  // if (m_detector) {
  //   PandoraGeometryMapper::registerGeometry(*m_pandora, *m_detector);
  // } else {
  //   warning("DD4hep detector not available; Pandora geometry not registered.");
  // }

  // Load algorithm settings from XML with parameter overrides
  if (!m_cfg.pandoraSettingsFile.empty()) {
    // Read the XML settings file
    std::ifstream xmlFile(m_cfg.pandoraSettingsFile);
    if (!xmlFile.is_open()) {
      warning("Could not open Pandora settings file '{}'; running with no algorithms loaded.",
              m_cfg.pandoraSettingsFile);
    } else {
      std::stringstream xmlBuffer;
      xmlBuffer << xmlFile.rdbuf();
      xmlFile.close();
      std::string xmlContent = xmlBuffer.str();

      // Apply parameter overrides from configuration
      std::string modifiedXml = applyXmlParameterOverrides(xmlContent, m_cfg);

      // Write modified XML to temporary file
      std::filesystem::path tempPath =
          std::filesystem::temp_directory_path() /
          ("pandora_settings_" + std::to_string(std::time(nullptr)) + ".xml");
      std::ofstream tempFile(tempPath);
      if (!tempFile.is_open()) {
        error("Could not create temporary XML file '{}'; using original settings file",
              tempPath.string());
        const pandora::StatusCode sc =
            PandoraApi::ReadSettings(*m_pandora, m_cfg.pandoraSettingsFile);
        if (sc != pandora::STATUS_CODE_SUCCESS) {
          warning("Could not read Pandora settings file '{}'; running with no algorithms loaded.",
                  m_cfg.pandoraSettingsFile);
        }
      } else {
        tempFile << modifiedXml;
        tempFile.close();

        // Log which parameters are being overridden
        logParameterOverrides();

        // Load the modified XML
        const pandora::StatusCode sc = PandoraApi::ReadSettings(*m_pandora, tempPath.string());

        // Clean up temporary file
        std::filesystem::remove(tempPath);

        if (sc != pandora::STATUS_CODE_SUCCESS) {
          warning("Could not read Pandora settings; running with no algorithms loaded.");
        } else {
          info("Loaded Pandora settings with parameter overrides from '{}'",
               m_cfg.pandoraSettingsFile);
        }
      }
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
  // Lazy initialization on first call
  std::call_once(m_initOnce, [this]() { initializePandora(); });

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

void PandoraPFA::logParameterOverrides() const {
  std::vector<std::string> overrides;

  // Topological clustering
  if (m_cfg.topologicalMaxCaloHitSeparation > 0) {
    overrides.push_back("topologicalMaxCaloHitSeparation=" +
                        std::to_string(m_cfg.topologicalMaxCaloHitSeparation));
  }
  if (m_cfg.topologicalMaxClusterSeparation > 0) {
    overrides.push_back("topologicalMaxClusterSeparation=" +
                        std::to_string(m_cfg.topologicalMaxClusterSeparation));
  }
  if (m_cfg.topologicalMaxClusterCosAngle > 0) {
    overrides.push_back("topologicalMaxClusterCosAngle=" +
                        std::to_string(m_cfg.topologicalMaxClusterCosAngle));
  }

  // Track-cluster association
  if (m_cfg.trackClusterMaxCaloHitSeparation > 0) {
    overrides.push_back("trackClusterMaxCaloHitSeparation=" +
                        std::to_string(m_cfg.trackClusterMaxCaloHitSeparation));
  }
  if (m_cfg.trackClusterMaxSeparationFromTrack > 0) {
    overrides.push_back("trackClusterMaxSeparationFromTrack=" +
                        std::to_string(m_cfg.trackClusterMaxSeparationFromTrack));
  }

  // Neutral PFO creation
  if (m_cfg.neutralPfoMinClusterEnergy > 0) {
    overrides.push_back("neutralPfoMinClusterEnergy=" +
                        std::to_string(m_cfg.neutralPfoMinClusterEnergy));
  }

  // Arbor-specific
  if (m_cfg.arborCellThresholdForRemoval > 0) {
    overrides.push_back("arborCellThresholdForRemoval=" +
                        std::to_string(m_cfg.arborCellThresholdForRemoval));
  }
  if (m_cfg.arborMaxSearchLayer > 0) {
    overrides.push_back("arborMaxSearchLayer=" + std::to_string(m_cfg.arborMaxSearchLayer));
  }
  if (m_cfg.arborMaxTransverseCellLengthMultiplier > 0) {
    overrides.push_back("arborMaxTransverseCellLengthMultiplier=" +
                        std::to_string(m_cfg.arborMaxTransverseCellLengthMultiplier));
  }
  if (m_cfg.arborShouldMergeIsolatedTrees >= 0) {
    overrides.push_back("arborShouldMergeIsolatedTrees=" +
                        std::to_string(m_cfg.arborShouldMergeIsolatedTrees));
  }
  if (m_cfg.arborIsolatedTreeEnergyCutForMerging > 0) {
    overrides.push_back("arborIsolatedTreeEnergyCutForMerging=" +
                        std::to_string(m_cfg.arborIsolatedTreeEnergyCutForMerging));
  }
  if (m_cfg.arborMinClusterEnergyForMerging > 0) {
    overrides.push_back("arborMinClusterEnergyForMerging=" +
                        std::to_string(m_cfg.arborMinClusterEnergyForMerging));
  }
  if (m_cfg.arborUseShowerProfile >= 0) {
    overrides.push_back("arborUseShowerProfile=" + std::to_string(m_cfg.arborUseShowerProfile));
  }

  if (!overrides.empty()) {
    info("Pandora parameter overrides active:");
    for (const auto& override : overrides) {
      info("  - {}", override);
    }
  } else {
    debug("No Pandora parameter overrides; using XML defaults");
  }
}

} // namespace eicrecon
