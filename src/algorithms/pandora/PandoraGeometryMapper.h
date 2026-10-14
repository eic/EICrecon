// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 EICrecon authors

#pragma once

#include <Api/PandoraApi.h>
#include <DD4hep/Detector.h>
#include <DDRec/DetectorData.h>
#include <Pandora/Pandora.h>
#include <string>
#include <vector>

namespace eicrecon {

/// Translates DD4hep detector geometry into PandoraSDK sub-detector descriptions.
///
/// Call registerGeometry() once during job initialisation (after the DD4hep geometry
/// is fully loaded).  Each call to registerGeometry() registers all relevant
/// sub-detectors (ECAL barrel/endcap, HCAL barrel/endcap) with the supplied
/// pandora::Pandora instance.
class PandoraGeometryMapper {
public:
  /// Register all EIC sub-detectors with @p pandora using geometry from @p detector.
  ///
  /// @param pandora   The Pandora instance to register sub-detectors with.
  /// @param detector  The fully-loaded DD4hep detector description.
  static void registerGeometry(pandora::Pandora& pandora, const dd4hep::Detector& detector);

private:
  /// Register a single cylindrical sub-detector section.
  ///
  /// @param pandora       The Pandora instance.
  /// @param subDetName    Unique name string for the sub-detector.
  /// @param subDetType    Pandora sub-detector type enum value.
  /// @param innerR        Inner radius [mm].
  /// @param outerR        Outer radius [mm].
  /// @param innerZ        Minimum |z| extent [mm].
  /// @param outerZ        Maximum |z| extent [mm].
  /// @param nLayers       Number of sampling layers.
  /// @param layerData     DDRec layer data with per-layer material budgets.
  static void registerSubDetector(pandora::Pandora& pandora, const std::string& subDetName,
                                  pandora::SubDetectorType subDetType, float innerR, float outerR,
                                  float innerZ, float outerZ, unsigned int nLayers,
                                  const dd4hep::rec::LayeredCalorimeterData& layerData);
}; // end PandoraGeometryMapper

} // namespace eicrecon
