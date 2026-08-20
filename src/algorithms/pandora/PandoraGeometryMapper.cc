// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 EICrecon authors

#include "PandoraGeometryMapper.h"

#include <DD4hep/DetElement.h>
#include <DD4hep/Volumes.h>
#include <DDRec/DetectorData.h>
#include <Pandora/PandoraEnumeratedTypes.h>

#include <stdexcept>

namespace eicrecon {

void PandoraGeometryMapper::registerGeometry(pandora::Pandora& pandora,
                                             const dd4hep::Detector& detector) {
  // Mapping of DD4hep detector names (as used in EIC detector descriptions) to
  // Pandora sub-detector types.  The names here follow the ePIC detector XML
  // convention; adjust if a different detector variant is used.
  struct SubDetSpec {
    const char* dd4hepName;
    pandora::SubDetectorType pandoraType;
    const char* pandoraName;
  };

  static const SubDetSpec kSubDetSpecs[] = {
      {"EcalBarrel", pandora::ECAL_BARREL, "ECalBarrel"},
      {"EcalEndcapN", pandora::ECAL_ENDCAP, "ECalEndcapN"},
      {"EcalEndcapP", pandora::ECAL_ENDCAP, "ECalEndcapP"},
      {"HcalBarrel", pandora::HCAL_BARREL, "HCalBarrel"},
      {"HcalEndcapN", pandora::HCAL_ENDCAP, "HCalEndcapN"},
      {"HcalEndcapP", pandora::HCAL_ENDCAP, "HCalEndcapP"},
  };

  for (const auto& spec : kSubDetSpecs) {
    const auto& detectors = detector.detectors();
    auto it               = detectors.find(spec.dd4hepName);
    if (it == detectors.end()) {
      // Sub-detector not present in this geometry variant — skip silently.
      continue;
    }

    const dd4hep::DetElement& det                        = it->second;
    const dd4hep::rec::LayeredCalorimeterData* layerData = nullptr;
    try {
      layerData = det.extension<dd4hep::rec::LayeredCalorimeterData>();
    } catch (...) {
      // Extension not available; skip.
      continue;
    }
    if (!layerData || layerData->layers.empty()) {
      continue;
    }

    // Derive overall envelope from the layer data.
    const float innerR         = static_cast<float>(layerData->extent[0] / dd4hep::mm);
    const float outerR         = static_cast<float>(layerData->extent[1] / dd4hep::mm);
    const float innerZ         = static_cast<float>(layerData->extent[2] / dd4hep::mm);
    const float outerZ         = static_cast<float>(layerData->extent[3] / dd4hep::mm);
    const unsigned int nLayers = static_cast<unsigned int>(layerData->layers.size());

    // Average layer thickness (distance_sensitive field gives absorber+sensitive thickness).
    float totalThickness = 0.f;
    for (const auto& layer : layerData->layers) {
      totalThickness +=
          static_cast<float>((layer.inner_thickness + layer.outer_thickness) / dd4hep::mm);
    }
    const float layerThickness = (nLayers > 0) ? totalThickness / static_cast<float>(nLayers) : 0.f;

    registerSubDetector(pandora, spec.pandoraName, spec.pandoraType, innerR, outerR, innerZ, outerZ,
                        nLayers, *layerData);
  }
}

void PandoraGeometryMapper::registerSubDetector(
    pandora::Pandora& pandora, const std::string& subDetName, pandora::SubDetectorType subDetType,
    float innerR, float outerR, float innerZ, float outerZ, unsigned int nLayers,
    const dd4hep::rec::LayeredCalorimeterData& layerData) {
  object_creation::Geometry::SubDetectorParameters subDetParams;
  subDetParams.m_subDetectorName.Set(subDetName);
  subDetParams.m_subDetectorType.Set(subDetType);
  subDetParams.m_innerRCoordinate.Set(innerR);
  subDetParams.m_outerRCoordinate.Set(outerR);
  subDetParams.m_innerZCoordinate.Set(innerZ);
  subDetParams.m_outerZCoordinate.Set(outerZ);
  subDetParams.m_innerPhiCoordinate.Set(0.f);
  subDetParams.m_outerPhiCoordinate.Set(0.f);
  subDetParams.m_innerSymmetryOrder.Set(0);
  subDetParams.m_outerSymmetryOrder.Set(0);
  // Barrel sub-detectors are mirrored in z; endcaps are not.
  subDetParams.m_isMirroredInZ.Set(
      (subDetType == pandora::ECAL_BARREL || subDetType == pandora::HCAL_BARREL));
  subDetParams.m_nLayers.Set(nLayers);

  // Populate layer parameters with per-layer material budgets from DDRec.
  float cumulativeRadLen = 0.f;
  float cumulativeIntLen = 0.f;

  // Radiation and interaction length estimates for common materials.
  // These are approximations; actual values depend on the material composition.
  // Lead: X0 ~ 0.56 cm, lambda ~ 18.0 cm
  // Iron: X0 ~ 1.76 cm, lambda ~ 16.8 cm
  // Scintillator (polystyrene): X0 ~ 42 cm, lambda ~ 80 cm

  for (unsigned int iLayer = 0; iLayer < nLayers && iLayer < layerData.layers.size(); ++iLayer) {
    const auto& layer = layerData.layers[iLayer];
    object_creation::Geometry::LayerParameters layerParams;
    layerParams.m_closestDistanceToIp.Set(
        innerR +
        (static_cast<float>(iLayer) + 0.5f) *
            static_cast<float>((layer.inner_thickness + layer.outer_thickness) / dd4hep::mm));

    // Extract absorber thickness for material budget calculation.
    // absorberThickness is in cm; convert to radiation/interaction lengths.
    const float absorberThickness = static_cast<float>(layer.absorberThickness / dd4hep::cm);

    float dRadLen = 0.f;
    float dIntLen = 0.f;

    // Estimate material composition based on detector type and set accordingly.
    if (subDetType == pandora::ECAL_BARREL || subDetType == pandora::ECAL_ENDCAP) {
      // ECAL typically lead/tungsten + scintillator.
      // Assume lead-like absorber: X0 ~ 0.56 cm, lambda ~ 18 cm
      if (absorberThickness > 0.f) {
        dRadLen = absorberThickness / 0.56f; // X0 for lead
        dIntLen = absorberThickness / 18.0f; // lambda for lead
      }
    } else if (subDetType == pandora::HCAL_BARREL || subDetType == pandora::HCAL_ENDCAP) {
      // HCAL typically iron + scintillator.
      // Assume iron absorber: X0 ~ 1.76 cm, lambda ~ 16.8 cm
      if (absorberThickness > 0.f) {
        dRadLen = absorberThickness / 1.76f; // X0 for iron
        dIntLen = absorberThickness / 16.8f; // lambda for iron
      }
    }

    // Fall back to rough estimates if absorber thickness is not available.
    if (dRadLen < 0.001f && dIntLen < 0.001f) {
      dRadLen =
          (subDetType == pandora::ECAL_BARREL || subDetType == pandora::ECAL_ENDCAP) ? 0.5f : 0.05f;
      dIntLen =
          (subDetType == pandora::HCAL_BARREL || subDetType == pandora::HCAL_ENDCAP) ? 0.1f : 0.01f;
    }

    cumulativeRadLen += dRadLen;
    cumulativeIntLen += dIntLen;
    layerParams.m_nRadiationLengths.Set(cumulativeRadLen);
    layerParams.m_nInteractionLengths.Set(cumulativeIntLen);
    subDetParams.m_layerParametersVector.push_back(layerParams);
  }

  PANDORA_THROW_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=,
                          PandoraApi::Geometry::SubDetector::Create(pandora, subDetParams));
}

} // namespace eicrecon
