// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minjung Kim, Joshua Sobaljic, Shujie Li
//
// RandomNoisePixel caches the discrete addressable pixels of each sensitive
// component in compact ranges, then samples noise from one global per-pixel rate.

#include "RandomNoisePixel.h"

#include <DD4hep/Alignments.h>
#include <DD4hep/Segmentations.h>
#include <DD4hep/Volumes.h>
#include <DDSegmentation/CartesianGridXY.h>
#include <DDSegmentation/CartesianGridXZ.h>
#include <DDSegmentation/CylindricalGridPhiZ.h>
#include <DDSegmentation/MultiSegmentation.h>
#include <DDSegmentation/Segmentation.h>
#include <TGeoBBox.h>
#include <TGeoNode.h>
#include <TGeoShape.h>
#include <TGeoTube.h>
#include <algorithms/geo.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <iterator>
#include <limits>
#include <mutex>
#include <numbers>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <utility>

namespace eicrecon {
namespace {

  using RawSegmentation = dd4hep::DDSegmentation::Segmentation;

  // ROOT's geometry navigator is shared. This mutex protects only the short
  // initialization phase in which global positions are converted to cell IDs.
  std::mutex& geometryInitializationMutex() {
    static std::mutex mutex;
    return mutex;
  }

  // Make DD4hep volume-ID field matching insensitive to capitalization.
  std::string lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
  }

  // Read the layer number from a placed volume's explicit DD4hep volume IDs.
  std::optional<int> layerFromPlacement(const dd4hep::PlacedVolume& placement) {
    if (!placement.isValid()) {
      return std::nullopt;
    }
    for (const auto& [field, value] : placement.volIDs()) {
      const auto name = lower(field);
      if (name == "layer" || name.find("lay") != std::string::npos) {
        return value;
      }
    }
    return std::nullopt;
  }

  // Prefer the layer placement ID, then try the module placement for geometries
  // that attach the layer field one level lower in the hierarchy.
  std::optional<int> layerFromDetElement(const dd4hep::DetElement& layer,
                                         const dd4hep::DetElement& module) {
    if (auto id = layerFromPlacement(layer.placement())) {
      return id;
    }
    return layerFromPlacement(module.placement());
  }

  // Copy a placed volume's local-to-parent transformation into an owned matrix.
  TGeoHMatrix placementTransform(const dd4hep::PlacedVolume& placement) {
    if (auto* node = placement.ptr()) {
      if (auto* matrix = node->GetMatrix()) {
        return TGeoHMatrix{*matrix};
      }
    }
    return TGeoHMatrix{};
  }

  // Compose child-local -> parent-local with parent-local -> world.
  TGeoHMatrix composeTransforms(const TGeoHMatrix& parentToWorld,
                                const TGeoHMatrix& childToParent) {
    TGeoHMatrix childToWorld{parentToWorld};
    childToWorld.Multiply(childToParent);
    return childToWorld;
  }

  // Accept a sensitive volume only when its readout exactly matches this
  // RandomNoisePixel instance (for example, SiBarrelHits but not MPGD hits).
  bool hasRequestedReadout(const dd4hep::PlacedVolume& placement, std::string_view readoutName) {
    if (!placement.isValid()) {
      return false;
    }
    dd4hep::SensitiveDetector sensitive{placement.volume().sensitiveDetector()};
    if (!sensitive.isValid()) {
      return false;
    }
    const auto readout = sensitive.readout();
    return readout.isValid() && readout.name() == readoutName;
  }

  // Record the shape and transform needed to identify one sensitive placement.
  // This initialization-only information is discarded after the cache is built.
  RandomNoisePixel::SensitiveComponentGeometry
  makeComponentGeometry(const std::string& detectorName, int layer,
                        const dd4hep::PlacedVolume& placement, const TGeoHMatrix& localToWorld) {
    auto volume = placement.volume();
    if (!volume.isValid() || !volume.ptr() || !volume->GetShape()) {
      throw std::runtime_error("RandomNoisePixel sensitive volume has no TGeo shape");
    }
    return {.detectorName          = detectorName,
            .layer                 = layer,
            .volume                = volume.ptr(),
            .localToWorldTransform = localToWorld};
  }

  // Recursively search a module assembly for sensitive descendants while
  // accumulating the complete sensitive-local to world transformation.
  void
  collectSensitiveDescendants(const std::string& detectorName, int layer,
                              const dd4hep::PlacedVolume& parent, const TGeoHMatrix& parentToWorld,
                              std::string_view readoutName,
                              std::vector<RandomNoisePixel::SensitiveComponentGeometry>& output) {
    auto* node = parent.ptr();
    if (!node) {
      return;
    }

    for (int i = 0; i < node->GetNdaughters(); ++i) {
      auto* daughterNode = node->GetDaughter(i);
      if (!daughterNode) {
        continue;
      }
      dd4hep::PlacedVolume daughter{daughterNode};
      const auto daughterToWorld = composeTransforms(parentToWorld, placementTransform(daughter));
      if (hasRequestedReadout(daughter, readoutName)) {
        output.push_back(makeComponentGeometry(detectorName, layer, daughter, daughterToWorld));
      } else {
        collectSensitiveDescendants(detectorName, layer, daughter, daughterToWorld, readoutName,
                                    output);
      }
    }
  }

  // Return all matching sensitive pieces belonging to one module. Some modules
  // are directly sensitive; others contain two or more sensitive daughters.
  std::vector<RandomNoisePixel::SensitiveComponentGeometry>
  findSensitiveComponents(const std::string& detectorName, int layer,
                          const dd4hep::DetElement& module, std::string_view readoutName) {
    const auto modulePlacement = module.placement();
    if (!modulePlacement.isValid()) {
      return {};
    }

    std::vector<RandomNoisePixel::SensitiveComponentGeometry> components;
    const auto moduleToWorld = module.nominal().worldTransformation();
    if (hasRequestedReadout(modulePlacement, readoutName)) {
      components.push_back(
          makeComponentGeometry(detectorName, layer, modulePlacement, moduleToWorld));
    } else {
      collectSensitiveDescendants(detectorName, layer, modulePlacement, moduleToWorld, readoutName,
                                  components);
    }
    return components;
  }

  // Access the axis-aligned local bounds supplied by ROOT for the sensor shape.
  const TGeoBBox& boundingBox(const RandomNoisePixel::SensitiveComponentGeometry& component) {
    const auto* box = dynamic_cast<const TGeoBBox*>(component.volume->GetShape());
    if (!box) {
      throw std::runtime_error("RandomNoisePixel requires a TGeo shape with bounding-box data");
    }
    return *box;
  }

  // Choose a point safely inside the sensor. Tube segments require their radial
  // and angular midpoint because the center of their bounding box can be empty.
  std::array<double, 3>
  referenceLocalPoint(const RandomNoisePixel::SensitiveComponentGeometry& component) {
    if (const auto* tube = dynamic_cast<const TGeoTubeSeg*>(component.volume->GetShape())) {
      const double radius = 0.5 * (tube->GetRmin() + tube->GetRmax());
      const double phi    = 0.5 * (tube->GetPhi1() + tube->GetPhi2()) * std::numbers::pi / 180.0;
      return {radius * std::cos(phi), radius * std::sin(phi), 0.0};
    }

    const auto& box    = boundingBox(component);
    const auto* origin = box.GetOrigin();
    return {origin[0], origin[1], origin[2]};
  }

  // Resolve a MultiSegmentation to the concrete grid selected by fields already
  // present in the sensor's base volume ID (the BVTX discriminator is layer).
  const RawSegmentation& selectedSegmentation(const dd4hep::Segmentation& segmentation,
                                              std::uint64_t baseVolumeID) {
    const RawSegmentation* selected = segmentation.segmentation();
    while (selected && selected->type() == "MultiSegmentation") {
      const auto* multi = dynamic_cast<const dd4hep::DDSegmentation::MultiSegmentation*>(selected);
      if (!multi) {
        throw std::runtime_error("RandomNoisePixel cannot access MultiSegmentation");
      }
      selected = &multi->subsegmentation(baseVolumeID);
    }
    if (!selected) {
      throw std::runtime_error("RandomNoisePixel readout has no segmentation");
    }
    return *selected;
  }

  // Convert a physical coordinate interval into the inclusive integer indices
  // of pixel centers inside it, then clamp to the cell-ID field's legal range.
  std::pair<std::int64_t, std::int64_t>
  indexRange(double minimum, double maximum, double pitch, double offset,
             const dd4hep::DDSegmentation::BitFieldElement& field) {
    if (!(pitch > 0.0) || !std::isfinite(pitch)) {
      throw std::runtime_error("RandomNoisePixel found an invalid segmentation pitch");
    }
    constexpr double indexTolerance = 1.0e-9;
    auto first = static_cast<std::int64_t>(std::ceil((minimum - offset) / pitch - indexTolerance));
    auto last  = static_cast<std::int64_t>(std::floor((maximum - offset) / pitch + indexTolerance));
    first      = std::max(first, static_cast<std::int64_t>(field.minValue()));
    last       = std::min(last, static_cast<std::int64_t>(field.maxValue()));
    return {first, last};
  }

  // A true box can be represented by one rectangular range without row tests.
  bool isPureBox(const TGeoShape& shape) {
    return std::string_view{shape.ClassName()} == "TGeoBBox";
  }

  // ROOT may classify a mathematically boundary-centered pixel as just outside
  // after a local -> global -> local floating-point round trip. Accept only
  // points within ROOT's own surface tolerance; larger misses remain errors.
  bool containsOrTouches(const TGeoShape& shape, const double point[3]) {
    return shape.Contains(point) || shape.Safety(point, false) <= 10.0 * TGeoShape::Tolerance();
  }

  // Build the exact discrete pixel layout for Cartesian XY or XZ grids. Boxes
  // become rectangles; trapezoids and clipped sensors become compact row spans.
  std::shared_ptr<RandomNoisePixel::PixelLayout>
  makeCartesianLayout(const RandomNoisePixel::SensitiveComponentGeometry& component,
                      RandomNoisePixel::GridKind kind, std::string firstField,
                      std::string secondField, double firstPitch, double secondPitch,
                      double firstOffset, double secondOffset,
                      const dd4hep::DDSegmentation::BitFieldCoder& decoder) {
    const auto& box           = boundingBox(component);
    const auto* origin        = box.GetOrigin();
    const bool xy             = kind == RandomNoisePixel::GridKind::CartesianXY;
    const double firstCenter  = origin[0];
    const double firstHalf    = box.GetDX();
    const double secondCenter = xy ? origin[1] : origin[2];
    const double secondHalf   = xy ? box.GetDY() : box.GetDZ();
    // Step 1: turn the sensor's physical bounds into candidate pixel-index bounds.
    const auto [firstMin, firstMax] = indexRange(firstCenter - firstHalf, firstCenter + firstHalf,
                                                 firstPitch, firstOffset, decoder[firstField]);
    const auto [secondMin, secondMax] =
        indexRange(secondCenter - secondHalf, secondCenter + secondHalf, secondPitch, secondOffset,
                   decoder[secondField]);

    if (firstMax < firstMin || secondMax < secondMin) {
      throw std::runtime_error("RandomNoisePixel sensitive component contains no pixel centers");
    }

    auto layout         = std::make_shared<RandomNoisePixel::PixelLayout>();
    layout->kind        = kind;
    layout->firstField  = std::move(firstField);
    layout->secondField = std::move(secondField);

    // Step 2a: a box contains every candidate pixel center, so store four limits.
    if (isPureBox(*component.volume->GetShape())) {
      layout->rectangular    = true;
      layout->firstMin       = firstMin;
      layout->firstMax       = firstMax;
      layout->secondMin      = secondMin;
      layout->secondMax      = secondMax;
      const auto firstCount  = static_cast<std::uint64_t>(firstMax - firstMin + 1);
      const auto secondCount = static_cast<std::uint64_t>(secondMax - secondMin + 1);
      if (firstCount > std::numeric_limits<std::uint64_t>::max() / secondCount) {
        throw std::overflow_error("RandomNoisePixel pixel count overflow");
      }
      layout->totalPixels = firstCount * secondCount;
      return layout;
    }

    // Step 2b: for a non-box shape, test pixel centers against TGeo::Contains().
    const auto* shape = component.volume->GetShape();
    auto contains     = [&](std::int64_t firstIndex, std::int64_t secondIndex) {
      const double first  = firstOffset + firstPitch * static_cast<double>(firstIndex);
      const double second = secondOffset + secondPitch * static_cast<double>(secondIndex);
      double local[3]     = {origin[0], origin[1], origin[2]};
      local[0]            = first;
      if (xy) {
        local[1] = second;
      } else {
        local[2] = second;
      }
      return containsOrTouches(*shape, local);
    };

    // Step 3: each supported non-box sensor is convex. Find one interior pixel
    // near the row center, then binary-search its left and right boundaries.
    const auto middleGuess = std::clamp(
        static_cast<std::int64_t>(std::llround((firstCenter - firstOffset) / firstPitch)), firstMin,
        firstMax);
    std::uint64_t cumulative = 0;
    for (std::int64_t second = secondMin; second <= secondMax; ++second) {
      if (!contains(middleGuess, second)) {
        continue;
      }

      std::int64_t low  = firstMin;
      std::int64_t high = middleGuess;
      while (low < high) {
        const auto middle = low + (high - low) / 2;
        if (contains(middle, second)) {
          high = middle;
        } else {
          low = middle + 1;
        }
      }
      const auto rowFirst = low;

      low  = middleGuess;
      high = firstMax;
      while (low < high) {
        const auto middle = low + (high - low + 1) / 2;
        if (contains(middle, second)) {
          low = middle;
        } else {
          high = middle - 1;
        }
      }
      const auto rowLast  = low;
      const auto rowCount = static_cast<std::uint64_t>(rowLast - rowFirst + 1);
      if (cumulative > std::numeric_limits<std::uint64_t>::max() - rowCount) {
        throw std::overflow_error("RandomNoisePixel pixel row count overflow");
      }
      // The cumulative count maps a flat random integer back to this row.
      cumulative += rowCount;
      layout->rows.push_back({second, rowFirst, rowLast, cumulative});
    }

    if (layout->rows.empty()) {
      throw std::runtime_error("RandomNoisePixel found no segmentation centers inside TGeo shape");
    }
    layout->totalPixels = cumulative;
    return layout;
  }

  // Build a phi-z rectangle in the sensitive volume's local coordinate system.
  // DD4hep later applies the placement transform when converting a cell ID to
  // a global position, so using world bounds here would apply the placement twice.
  std::shared_ptr<RandomNoisePixel::PixelLayout>
  makeCylindricalLayout(const RandomNoisePixel::SensitiveComponentGeometry& component,
                        const dd4hep::DDSegmentation::CylindricalGridPhiZ& grid,
                        const dd4hep::DDSegmentation::BitFieldCoder& decoder) {
    // Step 1: establish a local central phi so angles remain continuous across +/-pi.
    const auto center      = referenceLocalPoint(component);
    const double centerPhi = std::atan2(center[1], center[0]);

    double phiMin          = std::numeric_limits<double>::max();
    double phiMax          = std::numeric_limits<double>::lowest();
    double zMin            = std::numeric_limits<double>::max();
    double zMax            = std::numeric_limits<double>::lowest();
    auto includeLocalPoint = [&](double x, double y, double z) {
      const double phi =
          centerPhi + std::remainder(std::atan2(y, x) - centerPhi, 2.0 * std::numbers::pi);
      phiMin = std::min(phiMin, phi);
      phiMax = std::max(phiMax, phi);
      zMin   = std::min(zMin, z);
      zMax   = std::max(zMax, z);
    };

    // Step 2: inspect the true local TubeSeg boundaries. For other supported
    // shapes, use local bounding-box corners; initialization validation below
    // rejects a layout if the segmentation and solid frames are incompatible.
    if (const auto* tube = dynamic_cast<const TGeoTubeSeg*>(component.volume->GetShape())) {
      for (double radius : {tube->GetRmin(), tube->GetRmax()}) {
        for (double phiDegrees : {tube->GetPhi1(), tube->GetPhi2()}) {
          const double phi = phiDegrees * std::numbers::pi / 180.0;
          for (double z : {-tube->GetDz(), tube->GetDz()}) {
            includeLocalPoint(radius * std::cos(phi), radius * std::sin(phi), z);
          }
        }
      }
    } else {
      const auto& box    = boundingBox(component);
      const auto* origin = box.GetOrigin();
      for (double sx : {-1.0, 1.0}) {
        for (double sy : {-1.0, 1.0}) {
          for (double sz : {-1.0, 1.0}) {
            includeLocalPoint(origin[0] + sx * box.GetDX(), origin[1] + sy * box.GetDY(),
                              origin[2] + sz * box.GetDZ());
          }
        }
      }
    }

    // Step 3: convert angular and longitudinal bounds to discrete pixel indices.
    const auto [phiIndexMin, phiIndexMax] = indexRange(
        phiMin, phiMax, grid.gridSizePhi(), grid.offsetPhi(), decoder[grid.fieldNamePhi()]);
    const auto [zIndexMin, zIndexMax] =
        indexRange(zMin, zMax, grid.gridSizeZ(), grid.offsetZ(), decoder[grid.fieldNameZ()]);
    if (phiIndexMax < phiIndexMin || zIndexMax < zIndexMin) {
      throw std::runtime_error("RandomNoisePixel cylindrical component contains no pixels");
    }

    auto layout         = std::make_shared<RandomNoisePixel::PixelLayout>();
    layout->kind        = RandomNoisePixel::GridKind::CylindricalPhiZ;
    layout->firstField  = grid.fieldNamePhi();
    layout->secondField = grid.fieldNameZ();
    layout->rectangular = true;
    layout->firstMin    = phiIndexMin;
    layout->firstMax    = phiIndexMax;
    layout->secondMin   = zIndexMin;
    layout->secondMax   = zIndexMax;
    const auto phiCount = static_cast<std::uint64_t>(phiIndexMax - phiIndexMin + 1);
    const auto zCount   = static_cast<std::uint64_t>(zIndexMax - zIndexMin + 1);
    if (phiCount > std::numeric_limits<std::uint64_t>::max() / zCount) {
      throw std::overflow_error("RandomNoisePixel cylindrical pixel count overflow");
    }
    layout->totalPixels = phiCount * zCount;
    return layout;
  }

  // Map one flat index in [0, totalPixels) to the two segmentation indices.
  // This gives every addressable pixel exactly the same selection probability.
  std::pair<std::int64_t, std::int64_t> pixelIndices(const RandomNoisePixel::PixelLayout& layout,
                                                     std::uint64_t linearIndex) {
    if (linearIndex >= layout.totalPixels) {
      throw std::out_of_range("RandomNoisePixel linear pixel index is out of range");
    }
    if (layout.rectangular) {
      const auto firstCount = static_cast<std::uint64_t>(layout.firstMax - layout.firstMin + 1);
      return {layout.firstMin + static_cast<std::int64_t>(linearIndex % firstCount),
              layout.secondMin + static_cast<std::int64_t>(linearIndex / firstCount)};
    }

    const auto row =
        std::upper_bound(layout.rows.begin(), layout.rows.end(), linearIndex,
                         [](std::uint64_t value, const RandomNoisePixel::PixelRow& candidate) {
                           return value < candidate.cumulativeEnd;
                         });
    if (row == layout.rows.end()) {
      throw std::out_of_range("RandomNoisePixel could not resolve compact pixel row");
    }
    const auto previousEnd = row == layout.rows.begin() ? 0 : (row - 1)->cumulativeEnd;
    return {row->firstMin + static_cast<std::int64_t>(linearIndex - previousEnd), row->secondIndex};
  }

} // namespace

// Build all static geometry and pixel-count metadata before event processing.
void RandomNoisePixel::init() {
  // Step 1: reset state and validate the user-facing configuration.
  m_components.clear();
  m_layers.clear();

  if (!m_cfg.addNoise) {
    debug("RandomNoisePixel '{}': disabled by configuration; skipping geometry cache", name());
    return;
  }
  if (!(m_cfg.noise_rate_per_pixel_per_event >= 0.0 &&
        m_cfg.noise_rate_per_pixel_per_event <= 1.0)) {
    throw std::invalid_argument(
        "RandomNoisePixel noise_rate_per_pixel_per_event must be within [0, 1]");
  }

  // Step 2: CellIDPositionConverter::cellID(global) navigates shared TGeo state. JANA can
  // initialize detector factories concurrently, so serialize only cache construction.
  const std::scoped_lock geometryLock{geometryInitializationMutex()};

  // Step 3: obtain DD4hep services and the requested readout/segmentation.
  const auto& geo = algorithms::GeoSvc::instance();
  m_dd4hepGeo     = geo.detector();
  m_converter     = geo.cellIDPositionConverter();
  if (!m_dd4hepGeo || !m_converter) {
    throw std::runtime_error("RandomNoisePixel requires DD4hep geometry and cell-ID services");
  }

  m_readout = m_dd4hepGeo->readout(m_cfg.readout_name);
  if (!m_readout.isValid()) {
    throw std::invalid_argument("RandomNoisePixel invalid readout: " + m_cfg.readout_name);
  }

  // Step 4: collect geometry needed to identify each sensitive placement. This
  // vector owns the large TGeo transforms only during initialization.
  {
    std::vector<SensitiveComponentGeometry> componentGeometry;
    for (const auto& [name, detector] : m_dd4hepGeo->detectors()) {
      const auto sensitive = m_dd4hepGeo->sensitiveDetector(name);
      if (sensitive.isValid() && sensitive.readout().isValid() &&
          sensitive.readout().name() == m_cfg.readout_name) {
        collectDetectorComponents(detector, componentGeometry);
      }
    }

    // Step 5: convert initialization geometry into the compact event-time cache.
    cachePixelLayouts(componentGeometry);
  }

  // Step 6: the temporary transforms have now been released; construct layer totals.
  buildLayers();
  info("RandomNoisePixel '{}': cached {} sensitive components and {} layer groups for readout '{}'",
       name(), m_components.size(), m_layers.size(), m_cfg.readout_name);
}

// Enter each top-level layer of one detector system.
void RandomNoisePixel::collectDetectorComponents(
    const dd4hep::DetElement& detector,
    std::vector<SensitiveComponentGeometry>& componentGeometry) {
  for (const auto& [_, layer] : detector.children()) {
    collectLayerComponents(detector.name(), layer, componentGeometry);
  }
}

// Collect every sensitive placement below one detector layer.
void RandomNoisePixel::collectLayerComponents(
    const std::string& detectorName, const dd4hep::DetElement& layer,
    std::vector<SensitiveComponentGeometry>& componentGeometry) {
  for (const auto& [_, module] : layer.children()) {
    // Step 1: handle the common detector -> layer -> module hierarchy.
    const auto layerID = layerFromDetElement(layer, module);
    if (layerID) {
      auto components = findSensitiveComponents(detectorName, *layerID, module, m_cfg.readout_name);
      if (!components.empty()) {
        componentGeometry.insert(componentGeometry.end(),
                                 std::make_move_iterator(components.begin()),
                                 std::make_move_iterator(components.end()));
        continue;
      }
    }

    // Step 2: some geometries add one extra assembly level below the module.
    for (const auto& [__, child] : module.children()) {
      const auto childLayerID = layerID ? layerID : layerFromDetElement(layer, child);
      if (!childLayerID) {
        continue;
      }
      auto childComponents =
          findSensitiveComponents(detectorName, *childLayerID, child, m_cfg.readout_name);
      componentGeometry.insert(componentGeometry.end(),
                               std::make_move_iterator(childComponents.begin()),
                               std::make_move_iterator(childComponents.end()));
    }
  }
}

// Attach a base volume ID and a compact addressable-pixel layout to every component.
void RandomNoisePixel::cachePixelLayouts(
    const std::vector<SensitiveComponentGeometry>& componentGeometry) {
  // Step 1: get the generic segmentation handle and the cell-ID bit-field encoder.
  const auto segmentation = m_readout.segmentation();
  const auto* decoder     = m_readout.idSpec().decoder();
  if (!segmentation.isValid() || !decoder) {
    throw std::runtime_error("RandomNoisePixel readout has no segmentation or ID decoder");
  }

  // Repeated Cartesian sensors share a logical TGeo volume and segmentation.
  // Store one immutable layout for all such placements to keep memory O(components).
  struct SharedLayout {
    const TGeoVolume* volume            = nullptr;
    const RawSegmentation* segmentation = nullptr;
    std::shared_ptr<const PixelLayout> layout;
  };
  std::vector<SharedLayout> sharedCartesianLayouts;

  m_components.reserve(componentGeometry.size());
  for (const auto& geometry : componentGeometry) {
    // Step 2: map one known interior point to a full DD4hep cell ID, then clear
    // the local pixel fields to obtain the physical sensor's base volume ID.
    const auto reference = referenceLocalPoint(geometry);
    double local[3]      = {reference[0], reference[1], reference[2]};
    if (!geometry.volume->GetShape()->Contains(local)) {
      throw std::runtime_error("RandomNoisePixel reference point is outside sensitive volume '" +
                               std::string{geometry.volume->GetName()} + "' with shape '" +
                               geometry.volume->GetShape()->ClassName() + "'");
    }
    double globalCoordinates[3];
    geometry.localToWorldTransform.LocalToMaster(local, globalCoordinates);
    const dd4hep::Position global{globalCoordinates[0], globalCoordinates[1], globalCoordinates[2]};
    const auto referenceCell = m_converter->cellID(global);
    if (!m_converter->findContext(referenceCell)) {
      throw std::runtime_error("RandomNoisePixel could not resolve a component reference cell ID");
    }
    const auto baseVolumeID = segmentation.volumeID(referenceCell);
    SensitiveComponent component;
    component.detectorName = geometry.detectorName;
    component.layer        = geometry.layer;
    component.baseVolumeID = baseVolumeID;

    // Step 3: select the concrete grid and reuse a Cartesian layout when possible.
    const auto& selected  = selectedSegmentation(segmentation, baseVolumeID);
    const auto findShared = [&]() -> std::shared_ptr<const PixelLayout> {
      for (const auto& cached : sharedCartesianLayouts) {
        if (cached.volume == geometry.volume && cached.segmentation == &selected) {
          return cached.layout;
        }
      }
      return {};
    };

    // Step 4: dispatch only to segmentation types explicitly supported by the SVT.
    if (const auto* grid =
            dynamic_cast<const dd4hep::DDSegmentation::CartesianGridXY*>(&selected)) {
      component.layout = findShared();
      if (!component.layout) {
        component.layout = makeCartesianLayout(
            geometry, GridKind::CartesianXY, grid->fieldNameX(), grid->fieldNameY(),
            grid->gridSizeX(), grid->gridSizeY(), grid->offsetX(), grid->offsetY(), *decoder);
        sharedCartesianLayouts.push_back({geometry.volume, &selected, component.layout});
      }
    } else if (const auto* grid =
                   dynamic_cast<const dd4hep::DDSegmentation::CartesianGridXZ*>(&selected)) {
      component.layout = findShared();
      if (!component.layout) {
        component.layout = makeCartesianLayout(
            geometry, GridKind::CartesianXZ, grid->fieldNameX(), grid->fieldNameZ(),
            grid->gridSizeX(), grid->gridSizeZ(), grid->offsetX(), grid->offsetZ(), *decoder);
        sharedCartesianLayouts.push_back({geometry.volume, &selected, component.layout});
      }
    } else if (const auto* grid =
                   dynamic_cast<const dd4hep::DDSegmentation::CylindricalGridPhiZ*>(&selected)) {
      component.layout = makeCylindricalLayout(geometry, *grid, *decoder);
    } else {
      throw std::runtime_error("RandomNoisePixel unsupported segmentation type: " +
                               selected.type());
    }

    // Step 5: cache the count, then choose representative addresses across the
    // layout. Testing the edges as well as the middle catches placement/frame
    // mistakes that a single central pixel could hide.
    component.pixelCount                             = component.layout->totalPixels;
    std::array<std::uint64_t, 5> sampleLinearIndices = {
        0, component.pixelCount / 4, component.pixelCount / 2,
        component.pixelCount / 2 + component.pixelCount / 4, component.pixelCount - 1};
    std::sort(sampleLinearIndices.begin(), sampleLinearIndices.end());
    const auto uniqueEnd = std::unique(sampleLinearIndices.begin(), sampleLinearIndices.end());

    // Step 6: round-trip each cell ID through DD4hep and verify that its center
    // returns inside this exact placed sensitive solid. DD4hep applies the
    // placement once when producing the global position; MasterToLocal removes
    // that same complete (possibly nested) translation and rotation here.
    for (auto sample = sampleLinearIndices.begin(); sample != uniqueEnd; ++sample) {
      const auto [sampleFirst, sampleSecond] = pixelIndices(*component.layout, *sample);
      auto sampleID                          = component.baseVolumeID;
      decoder->set(sampleID, component.layout->firstField, sampleFirst);
      decoder->set(sampleID, component.layout->secondField, sampleSecond);
      if (!m_converter->findContext(sampleID)) {
        throw std::runtime_error("RandomNoisePixel generated an invalid sample cell ID");
      }

      const auto sampleGlobal           = m_converter->position(sampleID);
      double sampleGlobalCoordinates[3] = {sampleGlobal.x(), sampleGlobal.y(), sampleGlobal.z()};
      double sampleLocalCoordinates[3];
      geometry.localToWorldTransform.MasterToLocal(sampleGlobalCoordinates, sampleLocalCoordinates);
      if (!containsOrTouches(*geometry.volume->GetShape(), sampleLocalCoordinates)) {
        throw std::runtime_error(
            "RandomNoisePixel sample cell center does not return inside sensitive volume '" +
            std::string{geometry.volume->GetName()} + "' in detector '" + component.detectorName +
            "' layer " + std::to_string(component.layer) + "; pixel indices=(" +
            std::to_string(sampleFirst) + ", " + std::to_string(sampleSecond) +
            "), returned local position=(" + std::to_string(sampleLocalCoordinates[0]) + ", " +
            std::to_string(sampleLocalCoordinates[1]) + ", " +
            std::to_string(sampleLocalCoordinates[2]) +
            "); check segmentation coordinates and placement transforms");
      }
    }
    m_components.push_back(std::move(component));
  }
}

// Group components by detector and layer and prepare weighted random selection.
void RandomNoisePixel::buildLayers() {
  for (std::size_t componentIndex = 0; componentIndex < m_components.size(); ++componentIndex) {
    const auto& component = m_components[componentIndex];
    auto layer =
        std::find_if(m_layers.begin(), m_layers.end(), [&](const LayerGeometry& candidate) {
          return candidate.detectorName == component.detectorName &&
                 candidate.layer == component.layer;
        });
    if (layer == m_layers.end()) {
      m_layers.push_back({component.detectorName, component.layer, {}, {}, 0});
      layer = std::prev(m_layers.end());
    }
    if (layer->totalPixels > std::numeric_limits<std::uint64_t>::max() - component.pixelCount) {
      throw std::overflow_error("RandomNoisePixel layer pixel count overflow");
    }
    // Appending cumulative totals lets upper_bound select a component with
    // probability N_component / N_layer without visiting all components/event.
    layer->totalPixels += component.pixelCount;
    layer->componentIndices.push_back(componentIndex);
    layer->cumulativePixels.push_back(layer->totalPixels);
  }

  for (const auto& layer : m_layers) {
    info("RandomNoisePixel geometry: detector='{}' layer={} components={} pixels={} "
         "expected_hits={}",
         layer.detectorName, layer.layer, layer.componentIndices.size(), layer.totalPixels,
         m_cfg.noise_rate_per_pixel_per_event * static_cast<double>(layer.totalPixels));
  }
}

// Derive the event RNG seed from the run/event identity and algorithm name.
std::uint64_t
RandomNoisePixel::seedFromEventHeader(const edm4hep::EventHeaderCollection& headers) const {
  if (headers.empty()) {
    throw std::runtime_error("RandomNoisePixel requires a non-empty EventHeader collection");
  }
  return m_uid.getUniqueID(headers, name());
}

// Select one pixel uniformly within a sensitive component and encode its fields.
std::uint64_t RandomNoisePixel::randomCellID(const SensitiveComponent& component,
                                             std::mt19937_64& rng) const {
  std::uniform_int_distribution<std::uint64_t> pickPixel(0, component.pixelCount - 1);
  const auto [firstIndex, secondIndex] = pixelIndices(*component.layout, pickPixel(rng));
  auto cellID                          = component.baseVolumeID;
  const auto* decoder                  = m_readout.idSpec().decoder();
  decoder->set(cellID, component.layout->firstField, firstIndex);
  decoder->set(cellID, component.layout->secondField, secondIndex);
  return cellID;
}

// Generate noise for one layer using lambda = rate_per_pixel_per_event * N_pixels.
void RandomNoisePixel::addNoiseHitsForLayer(
    const LayerGeometry& layer, std::map<std::uint64_t, edm4eic::MutableRawTrackerHit>& hitMap,
    std::mt19937_64& rng) const {
  if (layer.totalPixels == 0 || m_cfg.noise_rate_per_pixel_per_event == 0.0) {
    return;
  }

  // Step 1: draw the total layer occupancy from the sparse-noise Poisson model.
  const double mean = m_cfg.noise_rate_per_pixel_per_event * static_cast<double>(layer.totalPixels);
  std::poisson_distribution<std::uint64_t> poisson(mean);
  const auto requested   = poisson(rng);
  const auto maxAttempts = std::max<std::uint64_t>(100, requested * 20);
  std::uniform_int_distribution<std::uint64_t> pickLayerPixel(0, layer.totalPixels - 1);

  std::uint64_t created  = 0;
  std::uint64_t attempts = 0;
  // Step 2: select a uniform layer-wide pixel. The cumulative component counts
  // first choose the sensor with probability proportional to its pixel count.
  while (created < requested && attempts < maxAttempts) {
    ++attempts;
    const auto selectedPixel     = pickLayerPixel(rng);
    const auto componentPosition = std::upper_bound(layer.cumulativePixels.begin(),
                                                    layer.cumulativePixels.end(), selectedPixel);
    const auto componentOffset =
        static_cast<std::size_t>(componentPosition - layer.cumulativePixels.begin());
    const auto& component = m_components[layer.componentIndices[componentOffset]];
    const auto cellID     = randomCellID(component, rng);
    // Step 3: electronic pixels can fire at most once in an event. At the default
    // occupancy, retries due to duplicates are extremely rare.
    if (hitMap.contains(cellID)) {
      continue;
    }

    // This algorithm models pixel occupancy, not the sensor pulse shape. Keep
    // the same placeholder charge and time convention as RandomNoiseModule.
    edm4eic::MutableRawTrackerHit hit;
    hit.setCellID(cellID);
    hit.setCharge(1.0e6);
    hit.setTimeStamp(0);
    hitMap.emplace(cellID, hit);
    ++created;
  }

  if (created < requested) {
    warning("RandomNoisePixel '{}': created {}/{} requested hits for detector '{}' layer {}",
            name(), created, requested, layer.detectorName, layer.layer);
  }
}

// Event entry point: seed, sample every layer, and emit deterministic cell-ID order.
void RandomNoisePixel::process(const Input& in, const Output& out) const {
  auto [outHits]       = out;
  const auto [headers] = in;

  if (!m_cfg.addNoise) {
    return;
  }
  if (!headers) {
    throw std::runtime_error("RandomNoisePixel requires an EventHeader collection");
  }
  if (m_layers.empty()) {
    throw std::runtime_error("RandomNoisePixel has no cached sensitive geometry");
  }

  // Step 1: make the random sequence depend on event identity, not thread scheduling.
  std::mt19937_64 rng(seedFromEventHeader(*headers));

  // Step 2: a sorted map removes duplicates and gives stable output ordering.
  std::map<std::uint64_t, edm4eic::MutableRawTrackerHit> noiseHits;
  for (const auto& layer : m_layers) {
    addNoiseHitsForLayer(layer, noiseHits, rng);
  }
  // Step 3: transfer the completed noise-only collection to PODIO.
  for (const auto& [_, hit] : noiseHits) {
    outHits->push_back(hit);
  }
}

} // namespace eicrecon
