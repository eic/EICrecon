// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 ePIC Collaboration

#pragma once

#include <Acts/Geometry/GeometryIdentifier.hpp>
#include <Acts/Geometry/TrackingGeometry.hpp>
#include <Acts/Surfaces/Surface.hpp>
#include <DD4hep/Detector.h>
#include <DD4hep/VolumeManager.h>
#include <spdlog/logger.h>
#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

// Version-dependent ACTS DD4hep include
#if __has_include(<ActsPlugins/DD4hep/DD4hepDetectorElement.hpp>)
#include <ActsPlugins/DD4hep/DD4hepDetectorElement.hpp>
using DD4hepDetElem = ActsPlugins::DD4hepDetectorElement;
#else
#include <Acts/Plugins/DD4hep/DD4hepDetectorElement.hpp>
using DD4hepDetElem = Acts::DD4hepDetectorElement;
#endif

namespace eicrecon {

/// Millepede alignment degrees of freedom (per layer, local sensor frame).
enum class AlignmentDOF : int {
  kTx   = 0, ///< Translation along local x [mm]
  kTy   = 1, ///< Translation along local y [mm]
  kTz   = 2, ///< Translation along local z [mm]
  kRx   = 3, ///< Rotation about local x [rad]
  kRy   = 4, ///< Rotation about local y [rad]
  kRz   = 5, ///< Rotation about local z [rad]
  kNDOF = 6,
};

/// Number of DOFs per alignable layer.
static constexpr int kAlignNDOF = static_cast<int>(AlignmentDOF::kNDOF);

/// Ordered list of silicon tracker layer path prefixes (DD4hep DetElement paths).
///
/// The order defines the Millepede label assignment:
///   label = layer_index * kAlignNDOF + dof_index + 1  (1-based positive integer)
///
/// Layer ordering matches calibrations/alignment/silicon_misalignment.xml.
/// Each entry is the *prefix* of the DetElement path so that individual
/// sensitive elements (modules, sensors) deeper in the hierarchy are matched
/// to their parent layer.
static constexpr std::array<std::string_view, 12> kSiliconLayerPaths = {{
    // Layer  0: labels  1 -  6
    "/world/MiddleSiBarrelSubAssembly/SagittaSiBarrel/SagittaSiBarrel_layer1",
    // Layer  1: labels  7 - 12
    "/world/OuterSiBarrelSubAssembly/OuterSiBarrel/OuterSiBarrel_layer2",
    // Layer  2: labels 13 - 18
    "/world/InnerSiTrackerSubAssembly/InnerTrackerEndcapP/InnerTrackerEndcapP_layer1_P",
    // Layer  3: labels 19 - 24
    "/world/InnerSiTrackerSubAssembly/InnerTrackerEndcapN/InnerTrackerEndcapN_layer1_N",
    // Layer  4: labels 25 - 30
    "/world/MiddleSiEndcapSubAssembly/MiddleTrackerEndcapP/MiddleTrackerEndcapP_layer1_P",
    // Layer  5: labels 31 - 36
    "/world/MiddleSiEndcapSubAssembly/MiddleTrackerEndcapN/MiddleTrackerEndcapN_layer1_N",
    // Layer  6: labels 37 - 42
    "/world/OuterSiEndcapSubAssembly/OuterTrackerEndcapP/OuterTrackerEndcapP_layer2_P",
    // Layer  7: labels 43 - 48
    "/world/OuterSiEndcapSubAssembly/OuterTrackerEndcapP/OuterTrackerEndcapP_layer3_P",
    // Layer  8: labels 49 - 54
    "/world/OuterSiEndcapSubAssembly/OuterTrackerEndcapP/OuterTrackerEndcapP_layer4_P",
    // Layer  9: labels 55 - 60
    "/world/OuterSiEndcapSubAssembly/OuterTrackerEndcapN/OuterTrackerEndcapN_layer2_N",
    // Layer 10: labels 61 - 66
    "/world/OuterSiEndcapSubAssembly/OuterTrackerEndcapN/OuterTrackerEndcapN_layer3_N",
    // Layer 11: labels 67 - 72
    "/world/OuterSiEndcapSubAssembly/OuterTrackerEndcapN/OuterTrackerEndcapN_layer4_N",
}};

/// Human-readable name for each layer (same order as kSiliconLayerPaths).
static constexpr std::array<std::string_view, 12> kSiliconLayerNames = {{
    "SagittaSiBarrel_layer1",
    "OuterSiBarrel_layer2",
    "InnerTrackerEndcapP_layer1_P",
    "InnerTrackerEndcapN_layer1_N",
    "MiddleTrackerEndcapP_layer1_P",
    "MiddleTrackerEndcapN_layer1_N",
    "OuterTrackerEndcapP_layer2_P",
    "OuterTrackerEndcapP_layer3_P",
    "OuterTrackerEndcapP_layer4_P",
    "OuterTrackerEndcapN_layer2_N",
    "OuterTrackerEndcapN_layer3_N",
    "OuterTrackerEndcapN_layer4_N",
}};

static constexpr std::size_t kNSiliconLayers = kSiliconLayerPaths.size();

/// Return the Millepede base label (DOF 0 label) for a given 0-based layer index.
/// All DOF labels for this layer are [baseLabel, baseLabel + kAlignNDOF - 1].
inline int siliconAlignmentBaseLabel(int layerIndex) { return layerIndex * kAlignNDOF + 1; }

/// Return the Millepede label for a specific (layer, DOF) pair.
inline int siliconAlignmentLabel(int layerIndex, AlignmentDOF dof) {
  return siliconAlignmentBaseLabel(layerIndex) + static_cast<int>(dof);
}

/// Decode a Millepede label back to (layerIndex, dof).
/// Returns std::nullopt if the label is out of range.
inline std::optional<std::pair<int, AlignmentDOF>> decodeSiliconAlignmentLabel(int label) {
  if (label < 1 || label > static_cast<int>(kNSiliconLayers) * kAlignNDOF) {
    return std::nullopt;
  }
  int zero_based = label - 1;
  int layerIndex = zero_based / kAlignNDOF;
  auto dof       = static_cast<AlignmentDOF>(zero_based % kAlignNDOF);
  return std::make_pair(layerIndex, dof);
}

/// Build a map from Acts::GeometryIdentifier (encoded as uint64_t) to the
/// 0-based silicon layer index by walking the ACTS TrackingGeometry and
/// matching each sensitive surface's DD4hep DetElement path against the
/// kSiliconLayerPaths prefixes.
///
/// The map is built once at algorithm initialization and queried per surface.
/// Returns only surfaces belonging to the 12 known silicon alignment layers.
inline std::unordered_map<std::uint64_t, int>
buildSiliconSurfaceLabelMap(const Acts::TrackingGeometry& trackingGeo,
                            const dd4hep::Detector& dd4hepDetector,
                            std::shared_ptr<spdlog::logger> log = nullptr) {
  std::unordered_map<std::uint64_t, int> surfaceToLayer;

  auto volman = dd4hepDetector.volumeManager();

  trackingGeo.visitSurfaces([&](const Acts::Surface* surface) {
    if (surface == nullptr) {
      return;
    }

#if Acts_VERSION_MAJOR >= 45
    const auto* det_element = dynamic_cast<const DD4hepDetElem*>(surface->surfacePlacement());
#else
    const auto* det_element =
        dynamic_cast<const DD4hepDetElem*>(surface->associatedDetectorElement());
#endif
    if (det_element == nullptr) {
      return;
    }

    auto* vol_ctx = volman.lookupContext(det_element->identifier());
    if (vol_ctx == nullptr) {
      return;
    }
    const std::string path = vol_ctx->element.path();

    // Find which silicon alignment layer this surface belongs to by
    // checking whether the path starts with one of the known layer prefixes.
    for (std::size_t i = 0; i < kNSiliconLayers; ++i) {
      if (path.rfind(kSiliconLayerPaths[i], 0) == 0) {
        const auto geoId = surface->geometryId().value();
        surfaceToLayer.emplace(geoId, static_cast<int>(i));
        if (log) {
          log->debug("SiliconAlignmentLabels: surface 0x{:016x} → layer {} ({}) path: {}", geoId, i,
                     kSiliconLayerNames[i], path);
        }
        break;
      }
    }
  });

  if (log) {
    log->info("SiliconAlignmentLabels: mapped {} surfaces to {} layers", surfaceToLayer.size(),
              kNSiliconLayers);
  }
  return surfaceToLayer;
}

} // namespace eicrecon
