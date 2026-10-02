// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minjung Kim, Joshua Sobaljic, Shujie Li

#pragma once

#include <DD4hep/DetElement.h>
#include <DD4hep/Detector.h>
#include <DD4hep/Readout.h>
#include <DDRec/CellIDPositionConverter.h>
#include <TGeoMatrix.h>
#include <TGeoVolume.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4hep/EventHeaderCollection.h>

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <string_view>
#include <vector>

#include "RandomNoisePixelConfig.h"
#include "algorithms/algorithm.h"
#include "algorithms/interfaces/UniqueIDGenSvc.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using RandomNoisePixelAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4hep::EventHeaderCollection>,
                          algorithms::Output<edm4eic::RawTrackerHitCollection>>;

class RandomNoisePixel : public RandomNoisePixelAlgorithm,
                         public WithPodConfig<RandomNoisePixelConfig> {
public:
  explicit RandomNoisePixel(std::string_view name)
      : RandomNoisePixelAlgorithm{
            name,
            {"EventHeader"},
            {"outputRawHitCollection"},
            "Generates noise-only RawTrackerHits from a global per-pixel occupancy."} {}

  // Read the static detector geometry once and build compact pixel lookup tables.
  void init();

  // Generate one event's noise hits from the cached geometry and the EventHeader seed.
  void process(const Input&, const Output&) const final;

  // The two pixel coordinates used by each supported DD4hep segmentation.
  enum class GridKind { CartesianXY, CartesianXZ, CylindricalPhiZ };

  // One contiguous run of valid pixels in a fixed second-coordinate row.
  // Example: for CartesianGridXZ, secondIndex is z and [firstMin, firstMax] is x.
  struct PixelRow {
    std::int64_t secondIndex    = 0;
    std::int64_t firstMin       = 0;
    std::int64_t firstMax       = -1;
    std::uint64_t cumulativeEnd = 0;
  };

  // Compact description of every addressable pixel in one sensor shape.
  // Rectangles need only four limits. Trapezoids use one PixelRow per valid row.
  struct PixelLayout {
    GridKind kind = GridKind::CartesianXY;
    std::string firstField;
    std::string secondField;
    bool rectangular       = false;
    std::int64_t firstMin  = 0;
    std::int64_t firstMax  = -1;
    std::int64_t secondMin = 0;
    std::int64_t secondMax = -1;
    std::vector<PixelRow> rows;
    std::uint64_t totalPixels = 0;
  };

  // Geometry needed only while constructing the persistent pixel cache in init().
  // Keeping it separate avoids retaining one TGeo transform throughout event processing.
  struct SensitiveComponentGeometry {
    std::string detectorName;
    int layer                = 0;
    const TGeoVolume* volume = nullptr;
    TGeoHMatrix localToWorldTransform;
  };

  // Event-time pixel metadata for one placed sensitive silicon component.
  // baseVolumeID identifies the physical sensor; layout supplies its pixel fields.
  struct SensitiveComponent {
    std::string detectorName;
    int layer                  = 0;
    std::uint64_t baseVolumeID = 0;
    std::shared_ptr<const PixelLayout> layout;
    std::uint64_t pixelCount = 0;
  };

  // All sensitive components belonging to one detector/layer sampling group.
  // cumulativePixels supports pixel-count-weighted component selection.
  struct LayerGeometry {
    std::string detectorName;
    int layer = 0;
    std::vector<std::size_t> componentIndices;
    std::vector<std::uint64_t> cumulativePixels;
    std::uint64_t totalPixels = 0;
  };

private:
  // Traverse the layer children of one DD4hep detector system.
  void collectDetectorComponents(const dd4hep::DetElement& detector,
                                 std::vector<SensitiveComponentGeometry>& componentGeometry);

  // Find sensitive module/component placements and preserve their detector layer IDs.
  void collectLayerComponents(const std::string& detectorName, const dd4hep::DetElement& layer,
                              std::vector<SensitiveComponentGeometry>& componentGeometry);

  // Determine each sensor's base cell ID and compact discrete pixel layout.
  void cachePixelLayouts(const std::vector<SensitiveComponentGeometry>& componentGeometry);

  // Group cached components by detector/layer and calculate cumulative pixel counts.
  void buildLayers();

  // Produce a deterministic seed from a required EventHeader collection.
  std::uint64_t seedFromEventHeader(const edm4hep::EventHeaderCollection& headers) const;

  // Select one addressable pixel from a component and encode its complete cell ID.
  std::uint64_t randomCellID(const SensitiveComponent& component, std::mt19937_64& rng) const;

  // Draw and create the Poisson-distributed noise hits for one detector layer.
  void addNoiseHitsForLayer(const LayerGeometry& layer,
                            std::map<std::uint64_t, edm4eic::MutableRawTrackerHit>& hitMap,
                            std::mt19937_64& rng) const;

  dd4hep::Readout m_readout;
  std::vector<SensitiveComponent> m_components;
  std::vector<LayerGeometry> m_layers;
  const dd4hep::Detector* m_dd4hepGeo                     = nullptr;
  const dd4hep::rec::CellIDPositionConverter* m_converter = nullptr;
  const algorithms::UniqueIDGenSvc& m_uid                 = algorithms::UniqueIDGenSvc::instance();
};

} // namespace eicrecon
