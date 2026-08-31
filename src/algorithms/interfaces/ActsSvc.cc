// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#include "ActsSvc.h"

#include <Acts/Definitions/Units.hpp>
#include <Acts/Material/IMaterialDecorator.hpp>
#include <Acts/Utilities/Logger.hpp>
#if __has_include(<ActsPlugins/Json/JsonMaterialDecorator.hpp>)
#include <ActsPlugins/Json/JsonMaterialDecorator.hpp>
#include <ActsPlugins/Json/MaterialMapJsonConverter.hpp>
#else
#include <Acts/Plugins/Json/JsonMaterialDecorator.hpp>
#include <Acts/Plugins/Json/MaterialMapJsonConverter.hpp>
#endif
#include <DD4hep/Detector.h>
#include <spdlog/common.h>
#include <stdexcept>
#include <utility>

#include "algorithms/tracking/ActsDD4hepDetector.h"
#include "algorithms/tracking/ActsDD4hepDetectorGen1.h"
#include "extensions/spdlog/SpdlogToActs.h"

// Check if Gen3 (Blueprint) support is available
#if __has_include(<ActsPlugins/DD4hep/BlueprintBuilder.hpp>)
#include "algorithms/tracking/ActsDD4hepDetectorGen3.h"

#define HAS_GEN3_SUPPORT 1
#else
#define HAS_GEN3_SUPPORT 0
#endif

namespace algorithms {

void ActsSvc::init(const dd4hep::Detector* dd4hep_detector) {
  try {
    if (!dd4hep_detector) {
      throw std::invalid_argument("DD4hep detector pointer cannot be null");
    }

    // Convert service log level for Acts
    const auto acts_log_level =
        eicrecon::SpdlogToActsLevel(static_cast<spdlog::level::level_enum>(level()));

    // Determine material map file path
    std::string materialMapPath = m_materialMap.value();
    if (materialMapPath.empty()) {
      // Try to read from DD4hep constant as fallback
      try {
        materialMapPath = dd4hep_detector->constant<std::string>("material-map");
        info("Using material map from DD4hep constant: '{}'", materialMapPath);
      } catch (const std::runtime_error&) {
        // DD4hep constant not found - use hardcoded fallback
        materialMapPath = "calibrations/materials-map.cbor";
        info("Using default material map path: '{}'", materialMapPath);
      }
    }

    // Load material decorator if specified
    std::shared_ptr<const Acts::IMaterialDecorator> materialDeco = nullptr;
    if (!materialMapPath.empty()) {
      info("Loading material map from file: '{}'", materialMapPath);
      try {
        // Set up the JSON converter config
        Acts::MaterialMapJsonConverter::Config jsonGeoConvConfig;
        // Note: could configure context here if needed

        // Create JSON-based material decorator
        materialDeco = std::make_shared<const Acts::JsonMaterialDecorator>(
            jsonGeoConvConfig, materialMapPath, acts_log_level);

        info("Material map loaded successfully");
      } catch (const std::exception& e) {
        error("Failed to load material map from '{}': {}", materialMapPath, e.what());
        throw;
      }
    }

    // Choose generation based on property
    int generation = m_generation.value();

    // Auto-detect if generation is 0
    if (generation == 0) {
#if HAS_GEN3_SUPPORT
      generation = 3;
      info("Auto-detecting Acts geometry generation: using Gen3 (Acts >= 44)");
#else
      generation = 1;
      info("Auto-detecting Acts geometry generation: using Gen1 by default");
#endif
    }

    if (generation == 1) {
      // Gen1: Explicit selection
      info("Creating Acts geometry using Gen1");

      eicrecon::ActsDD4hepDetectorGen1::Config cfg;
      cfg.setDD4hepDetector(dd4hep_detector);
      cfg.name                  = m_detectorName.value();
      cfg.logLevel              = Acts::Logging::INFO;
      cfg.materialDecorator     = materialDeco;
      cfg.layerEnvelopeR        = m_layerEnvelopeR.value() * Acts::UnitConstants::mm;
      cfg.layerEnvelopeZ        = m_layerEnvelopeZ.value() * Acts::UnitConstants::mm;
      cfg.defaultLayerThickness = m_defaultLayerThickness.value() * Acts::UnitConstants::mm;

      m_acts_detector = std::make_shared<eicrecon::ActsDD4hepDetectorGen1>(cfg);

    } else if (generation == 3) {
#if HAS_GEN3_SUPPORT
      // Gen3: Blueprint
      info("Creating Acts geometry using Gen3 (blueprint)");

      eicrecon::ActsDD4hepDetectorGen3::Config cfg;
      cfg.setDD4hepDetector(dd4hep_detector);
      cfg.name                   = m_detectorName.value();
      cfg.logLevel               = Acts::Logging::INFO;
      cfg.materialDecorator      = materialDeco;
      cfg.detectorElementFactory = eicrecon::ActsDD4hepDetectorGen3::defaultDetectorElementFactory;

      m_acts_detector = std::make_shared<eicrecon::ActsDD4hepDetectorGen3>(cfg);
#else
      throw std::runtime_error(
          "Gen3 (Blueprint) geometry requested but not available. "
          "This version of Acts does not include BlueprintBuilder support. "
          "Please use Generation=0 or Generation=1, or upgrade to Acts v46 or later.");
#endif

    } else {
      throw std::runtime_error("Invalid Acts generation: " + std::to_string(generation) +
                               " (must be 0, 1, or 3)");
    }

    info("Acts geometry created successfully with {} surfaces",
         m_acts_detector->surfaceMap().size());

  } catch (const std::exception& e) {
    error("Failed to create Acts geometry: {}", e.what());
    init(std::current_exception());
  } catch (...) {
    error("Failed to create Acts geometry: unknown exception");
    init(std::current_exception());
  }
}

void ActsSvc::init(std::shared_ptr<const eicrecon::ActsDD4hepDetector> acts_detector) {
  if (acts_detector) {
    info("ActsSvc initialized with pre-created detector");
    m_acts_detector = acts_detector;
  } else {
    throw std::invalid_argument("ActsSvc initialized with null detector");
  }
}

void ActsSvc::init(std::exception_ptr&& _failure) {
  failure = std::move(_failure);
  error("ActsSvc initialization failed - exception stored");
}

} // namespace algorithms
