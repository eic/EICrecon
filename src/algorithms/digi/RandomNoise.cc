// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minjung Kim, Joshua Sobaljic, Shujie Li
//
// RandomNoise generates noise-only RawTrackerHits by caching DD4hep module and
// sensitive-shape geometry once, then sampling random positions per configured
// detector layer and converting those positions to valid tracker cell IDs.

#include "RandomNoise.h"

#include <DD4hep/Volumes.h>
#include <TGeoMatrix.h>
#include <TGeoNode.h>
#include <algorithms/geo.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cctype>
#include <optional>
#include <sstream>
#include <stdexcept>

namespace eicrecon {
namespace {


  struct LocalTransform {
    std::array<double, 9> rotation{1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
    dd4hep::Position translation{0.0, 0.0, 0.0};
  };

  // Compact formatting for optional geometry diagnostics.
  std::string dimensionsToString(const std::vector<double>& dimensions) {
    std::ostringstream out;
    out << "[";
    for (std::size_t i = 0; i < dimensions.size(); ++i) {
      if (i != 0) {
        out << ", ";
      }
      out << dimensions[i];
    }
    out << "]";
    return out.str();
  }

  // Normalize DD4hep field names before matching layer-like volume IDs.
  std::string lower(std::string s) {
    for (auto& c : s) {
      c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return s;
  }

  // Last-resort layer extraction for geometries where the layer ID is encoded
  // in a DetElement name instead of a placed-volume ID.
  std::optional<int> firstIntegerIn(std::string_view text) {
    for (std::size_t i = 0; i < text.size(); ++i) {
      if (!std::isdigit(static_cast<unsigned char>(text[i]))) {
        continue;
      }
      int value = 0;
      while (i < text.size() && std::isdigit(static_cast<unsigned char>(text[i]))) {
        value = 10 * value + static_cast<int>(text[i] - '0');
        ++i;
      }
      return value;
    }
    return std::nullopt;
  }

  // Prefer explicit DD4hep volume IDs when identifying the detector layer.
  std::optional<int> layerFromPlacement(const dd4hep::PlacedVolume& pv) {
    if (!pv.isValid()) {
      return std::nullopt;
    }
    for (const auto& [field, value] : pv.volIDs()) {
      const auto name = lower(field);
      if (name == "layer" || name.find("lay") != std::string::npos) {
        return value;
      }
    }
    return std::nullopt;
  }

  // Resolve the layer ID from the layer DetElement first, then the module, then
  // name parsing. The name fallback keeps older or less regular geometries usable.
  std::optional<int> layerFromDetElement(const dd4hep::DetElement& layer,
                                         const dd4hep::DetElement& module) {
    if (auto id = layerFromPlacement(layer.placement())) {
      return id;
    }
    if (auto id = layerFromPlacement(module.placement())) {
      return id;
    }
    if (auto id = firstIntegerIn(layer.name())) {
      return id;
    }
    return firstIntegerIn(module.name());
  }

  // Read the local-to-parent placement transform from DD4hep/TGeo.
  LocalTransform placementTransform(const dd4hep::PlacedVolume& pv) {
    LocalTransform transform;
    if (auto* node = pv.ptr()) {
      if (auto* matrix = node->GetMatrix()) {
        if (const auto* rotation = matrix->GetRotationMatrix()) {
          for (std::size_t i = 0; i < transform.rotation.size(); ++i) {
            transform.rotation[i] = rotation[i];
          }
        }
        if (const auto* translation = matrix->GetTranslation()) {
          transform.translation = {translation[0], translation[1], translation[2]};
        }
      }
    }
    return transform;
  }

  // Apply a row-major rotation plus translation to a local point.
  dd4hep::Position applyTransform(const LocalTransform& transform, const dd4hep::Position& point) {
    const auto& rotation  = transform.rotation;
    const double local[3] = {point.x(), point.y(), point.z()};
    return {rotation[0] * local[0] + rotation[1] * local[1] + rotation[2] * local[2] +
                transform.translation.x(),
            rotation[3] * local[0] + rotation[4] * local[1] + rotation[5] * local[2] +
                transform.translation.y(),
            rotation[6] * local[0] + rotation[7] * local[1] + rotation[8] * local[2] +
                transform.translation.z()};
  }

  // Compose child-local -> parent-local with parent-local -> module-local.
  LocalTransform composeTransforms(const LocalTransform& parentToModule,
                                   const LocalTransform& childToParent) {
    LocalTransform childToModule;
    for (std::size_t row = 0; row < 3; ++row) {
      for (std::size_t col = 0; col < 3; ++col) {
        childToModule.rotation[3 * row + col] =
            parentToModule.rotation[3 * row + 0] * childToParent.rotation[0 * 3 + col] +
            parentToModule.rotation[3 * row + 1] * childToParent.rotation[1 * 3 + col] +
            parentToModule.rotation[3 * row + 2] * childToParent.rotation[2 * 3 + col];
      }
    }
    childToModule.translation = applyTransform(parentToModule, childToParent.translation);
    return childToModule;
  }

  // A placed volume is treated as a sensitive component only if its sensitive
  // detector readout exactly matches the readout configured for this algorithm.
  bool hasRequestedReadout(const dd4hep::PlacedVolume& pv, std::string_view readoutName) {
    if (!pv.isValid()) {
      return false;
    }
    dd4hep::SensitiveDetector sd{pv.volume().sensitiveDetector()};
    if (!sd.isValid()) {
      return false;
    }
    auto readout = sd.readout();
    return readout.isValid() && readout.name() == readoutName;
  }

  // Map DD4hep/ROOT shape type names to the sampling code supported below.
  std::optional<RandomNoise::SensitiveShapeKind> shapeKindFromName(std::string_view shapeName) {
    const auto name = lower(std::string{shapeName});
    if (name == "tgeobbox" || name == "box") {
      return RandomNoise::SensitiveShapeKind::Box;
    }
    if (name == "tgeotrd1" || name == "trd1") {
      return RandomNoise::SensitiveShapeKind::Trd1;
    }
    if (name == "tgeotrd2" || name == "trd2") {
      return RandomNoise::SensitiveShapeKind::Trd2;
    }
    if (name == "tgeotube" || name == "tube") {
      return RandomNoise::SensitiveShapeKind::Tube;
    }
    if (name == "tgeotubeseg" || name == "tgeotubs" || name == "tubesegment" || name == "tubs") {
      return RandomNoise::SensitiveShapeKind::TubeSegment;
    }
    if (name == "tgeocone" || name == "cone") {
      return RandomNoise::SensitiveShapeKind::Cone;
    }
    if (name == "tgeoeltu" || name == "eltu" || name == "ellipticaltube") {
      return RandomNoise::SensitiveShapeKind::EllipticalTube;
    }
    return std::nullopt;
  }

  // Check the DD4hep dimensions vector before any sampling code indexes it.
  bool hasRequiredDimensions(RandomNoise::SensitiveShapeKind shape, const std::vector<double>& d) {
    switch (shape) {
    case RandomNoise::SensitiveShapeKind::Box:
    case RandomNoise::SensitiveShapeKind::Tube:
    case RandomNoise::SensitiveShapeKind::EllipticalTube:
      return d.size() >= 3;
    case RandomNoise::SensitiveShapeKind::Trd1:
      return d.size() >= 4;
    case RandomNoise::SensitiveShapeKind::Trd2:
    case RandomNoise::SensitiveShapeKind::TubeSegment:
    case RandomNoise::SensitiveShapeKind::Cone:
      return d.size() >= 5;
    }
    return false;
  }

  // Estimate the sensitive solid volume used to weight multiple sensitive pieces
  // inside the same module. For thin sensors this is equivalent to area weighting
  // when all sensitive pieces have the same thickness.
  double sensitiveVolume(RandomNoise::SensitiveShapeKind shape, const std::vector<double>& d) {
    switch (shape) {
    case RandomNoise::SensitiveShapeKind::Box:
      return d.size() >= 3 ? 8.0 * d[0] * d[1] * d[2] : 0.0;
    case RandomNoise::SensitiveShapeKind::Trd1:
      return d.size() >= 4 ? 4.0 * d[2] * d[3] * (d[0] + d[1]) : 0.0;
    case RandomNoise::SensitiveShapeKind::Trd2: {
      if (d.size() < 5) {
        return 0.0;
      }
      const double meanAreaFactor = d[0] * d[2] +
                                    0.5 * (d[0] * (d[3] - d[2]) + d[2] * (d[1] - d[0])) +
                                    (d[1] - d[0]) * (d[3] - d[2]) / 3.0;
      return 8.0 * d[4] * meanAreaFactor;
    }
    case RandomNoise::SensitiveShapeKind::Tube:
      return d.size() >= 3 ? 2.0 * d[2] * M_PI * (d[1] * d[1] - d[0] * d[0]) : 0.0;
    case RandomNoise::SensitiveShapeKind::TubeSegment: {
      if (d.size() < 5) {
        return 0.0;
      }
      double phiWidth = d[4] - d[3];
      if (phiWidth < 0.0) {
        phiWidth += 2.0 * M_PI;
      }
      return d[2] * phiWidth * (d[1] * d[1] - d[0] * d[0]);
    }
    case RandomNoise::SensitiveShapeKind::Cone:
      if (d.size() >= 5) {
        const double outer = d[2] * d[2] + d[2] * d[4] + d[4] * d[4];
        const double inner = d[1] * d[1] + d[1] * d[3] + d[3] * d[3];
        return 2.0 * d[0] * M_PI * (outer - inner) / 3.0;
      }
      return 0.0;
    case RandomNoise::SensitiveShapeKind::EllipticalTube:
      return d.size() >= 3 ? 2.0 * d[2] * M_PI * d[0] * d[1] : 0.0;
    }
    return 0.0;
  }

  // Cache the sensitive solid shape and dimensions using DD4hep conventions.
  // In particular, DD4hep shape dimensions use radians for angular parameters.
  std::optional<RandomNoise::SensitiveComponent>
  componentFromPlacedVolume(const dd4hep::PlacedVolume& pv, const LocalTransform& localToModule) {
    if (!pv.isValid()) {
      return std::nullopt;
    }

    RandomNoise::SensitiveComponent component;
    component.localToModuleRotation    = localToModule.rotation;
    component.localToModuleTranslation = localToModule.translation;
    auto solid                         = pv.volume().solid();
    component.shapeName                = solid.type();

    auto shapeKind = shapeKindFromName(component.shapeName);
    if (!shapeKind) {
      return std::nullopt;
    }

    component.shape      = *shapeKind;
    component.dimensions = solid.dimensions();
    if (!hasRequiredDimensions(component.shape, component.dimensions)) {
      return std::nullopt;
    }
    component.samplingWeight =
        std::max(sensitiveVolume(component.shape, component.dimensions), 0.0);
    return component;
  }

  // Recursively collect sensitive descendants while preserving the full
  // sensitive-local to module-local transform, including future rotations.
  void collectSensitiveComponents(const dd4hep::PlacedVolume& parent,
                                  const LocalTransform& parentToModule,
                                  std::string_view readoutName,
                                  std::vector<RandomNoise::SensitiveComponent>& components) {
    TGeoNode* node = parent.ptr();
    if (!node) {
      return;
    }

    for (int i = 0; i < node->GetNdaughters(); ++i) {
      auto* daughterNode = node->GetDaughter(i);
      if (!daughterNode) {
        continue;
      }
      dd4hep::PlacedVolume daughter{daughterNode};
      const auto daughterToModule = composeTransforms(parentToModule, placementTransform(daughter));
      if (hasRequestedReadout(daughter, readoutName)) {
        if (auto component = componentFromPlacedVolume(daughter, daughterToModule)) {
          components.push_back(*component);
        }
        continue;
      }
      collectSensitiveComponents(daughter, daughterToModule, readoutName, components);
    }
  }

  // Find every sensitive component for a module. The module itself may be
  // sensitive, or it may contain multiple sensitive descendant volumes.
  std::vector<RandomNoise::SensitiveComponent>
  findSensitiveComponents(const dd4hep::DetElement& module, std::string_view readoutName) {
    auto modulePV = module.placement();
    if (!modulePV.isValid()) {
      return {};
    }

    if (hasRequestedReadout(modulePV, readoutName)) {
      auto component = componentFromPlacedVolume(modulePV, LocalTransform{});
      if (component) {
        return {*component};
      }
      return {};
    }

    std::vector<RandomNoise::SensitiveComponent> components;
    collectSensitiveComponents(modulePV, LocalTransform{}, readoutName, components);
    return components;
  }

  // Sanity check for the uniform-module assumption. This compares component
  // count, shape, and dimensions; it is not a full placement validation.
  bool sameSensitiveComponent(const RandomNoise::SensitiveComponent& lhs,
                              const RandomNoise::SensitiveComponent& rhs) {
    constexpr double tolerance = 1e-9;
    if (lhs.shape != rhs.shape || lhs.dimensions.size() != rhs.dimensions.size()) {
      return false;
    }
    if (lhs.shape == RandomNoise::SensitiveShapeKind::TubeSegment && lhs.dimensions.size() >= 5) {
      const auto phiWidth = [](const std::vector<double>& dimensions) {
        double width = dimensions[4] - dimensions[3];
        if (width < 0.0) {
          width += 2.0 * M_PI;
        }
        return width;
      };
      for (std::size_t i = 0; i < 3; ++i) {
        if (std::abs(lhs.dimensions[i] - rhs.dimensions[i]) > tolerance) {
          return false;
        }
      }
      return std::abs(phiWidth(lhs.dimensions) - phiWidth(rhs.dimensions)) <= tolerance;
    }
    for (std::size_t i = 0; i < lhs.dimensions.size(); ++i) {
      if (std::abs(lhs.dimensions[i] - rhs.dimensions[i]) > tolerance) {
        return false;
      }
    }
    return true;
  }

  // Compare module active layouts so uniform module sampling remains meaningful.
  bool sameSensitiveLayout(const RandomNoise::ModuleGeometry& lhs,
                           const RandomNoise::ModuleGeometry& rhs) {
    if (lhs.sensitiveComponents.size() != rhs.sensitiveComponents.size()) {
      return false;
    }
    for (std::size_t i = 0; i < lhs.sensitiveComponents.size(); ++i) {
      if (!sameSensitiveComponent(lhs.sensitiveComponents[i], rhs.sensitiveComponents[i])) {
        return false;
      }
    }
    return true;
  }

} // namespace

// Resolve DD4hep services, select detectors by readout name, and cache the
// geometry needed for event processing. Geometry is assumed static after init().
void RandomNoise::init() {
  m_modules.clear();
  m_layers.clear();

  if (!m_cfg.addNoise) {
    debug("RandomNoise '{}': disabled by configuration; skipping geometry cache", name());
    return;
  }

  const auto& geo = algorithms::GeoSvc::instance();
  m_dd4hepGeo     = geo.detector();
  m_converter     = geo.cellIDPositionConverter();

  if (!m_dd4hepGeo) {
    error("RandomNoise: no DD4hep geometry service found");
    return;
  }
  if (!m_converter) {
    error("RandomNoise: no CellIDPositionConverter found");
    return;
  }

  m_readout = m_dd4hepGeo->readout(m_cfg.readout_name);
  if (!m_readout.isValid()) {
    error("RandomNoise: invalid readout name '{}'", m_cfg.readout_name);
    return;
  }

  for (const auto& [name, det] : m_dd4hepGeo->detectors()) {
    auto sd = m_dd4hepGeo->sensitiveDetector(name);
    if (!sd.isValid()) {
      continue;
    }
    auto readout = sd.readout();
    if (readout.isValid() && readout.name() == m_cfg.readout_name) {
      collectDetectorModules(det);
    }
  }

  buildConfiguredLayers();

  info("RandomNoise '{}': cached {} modules in {} configured layer groups for readout '{}'", name(),
       m_modules.size(), m_layers.size(), m_cfg.readout_name);
}

// Traverse one detector system. The expected hierarchy is detector -> layers,
// with modules or module-like children below each layer.
void RandomNoise::collectDetectorModules(const dd4hep::DetElement& detector) {
  for (const auto& [_, layer] : detector.children()) {
    collectLayerModules(detector.name(), layer);
  }
}

// Cache modules for one layer. A module may contain multiple sensitive daughters,
// so all matching pieces are kept together under the same module placement.
void RandomNoise::collectLayerModules(const std::string& detectorName,
                                      const dd4hep::DetElement& layer) {
  for (const auto& [_, module] : layer.children()) {
    auto layerID    = layerFromDetElement(layer, module);
    auto components = findSensitiveComponents(module, m_cfg.readout_name);
    if (!layerID || components.empty()) {
      for (const auto& [__, child] : module.children()) {
        auto childLayerID    = layerID ? layerID : layerFromDetElement(layer, child);
        auto childComponents = findSensitiveComponents(child, m_cfg.readout_name);
        if (childLayerID && !childComponents.empty()) {
          m_modules.push_back({detectorName, *childLayerID, child, std::move(childComponents)});
        }
      }
      continue;
    }
    m_modules.push_back({detectorName, *layerID, module, std::move(components)});
  }
}

// Convert the configured per-layer means into cached layer groups. Uniform
// module sampling is valid only when modules in a group have equal active area.
void RandomNoise::buildConfiguredLayers() {
  const bool hasLayerConfig = !m_cfg.layer_id.empty() && !m_cfg.n_noise_hits_per_layer.empty() &&
                              m_cfg.layer_id.size() == m_cfg.n_noise_hits_per_layer.size();

  if (!hasLayerConfig) {
    if (!m_modules.empty()) {
      m_layers.push_back({"", -1, m_cfg.n_noise_hits_per_system, m_modules});
      warning("RandomNoise '{}': using system-wide noise mean {} because layer configuration is "
              "missing or inconsistent",
              name(), m_cfg.n_noise_hits_per_system);
    }
    return;
  }

  for (std::size_t i = 0; i < m_cfg.layer_id.size(); ++i) {
    const std::string detectorFilter =
        i < m_cfg.detector_names.size() ? m_cfg.detector_names[i] : std::string{};

    LayerGeometry layer{detectorFilter, m_cfg.layer_id[i], m_cfg.n_noise_hits_per_layer[i], {}};
    for (const auto& module : m_modules) {
      if (!detectorFilter.empty() && module.detectorName != detectorFilter) {
        continue;
      }
      if (module.layer == layer.layer) {
        layer.modules.push_back(module);
      }
    }

    if (layer.modules.empty()) {
      warning("RandomNoise '{}': no modules found for detector '{}' layer {}", name(),
              detectorFilter.empty() ? "<any>" : detectorFilter, layer.layer);
      continue;
    }

    const auto& reference      = layer.modules.front();
    const auto& firstComponent = reference.sensitiveComponents.front();
    debug("RandomNoise geometry cache: detector='{}' layer={} modules={} "
          "sensitive_components_per_module={} first_shape='{}' first_dd4hep_dimensions={}",
          detectorFilter.empty() ? layer.modules.front().detectorName : detectorFilter, layer.layer,
          layer.modules.size(), reference.sensitiveComponents.size(), firstComponent.shapeName,
          dimensionsToString(firstComponent.dimensions));
    for (const auto& module : layer.modules) {
      if (!sameSensitiveLayout(reference, module)) {
        warning("RandomNoise '{}': detector '{}' layer {} contains non-identical sensitive "
                "component layouts; uniform module sampling assumes equal active area",
                name(), detectorFilter.empty() ? module.detectorName : detectorFilter, layer.layer);
        break;
      }
    }
    m_layers.push_back(std::move(layer));
  }
}

// Seed from the EventHeader collection so noise remains reproducible across
// thread counts. The single-header pointer path is kept as a compatibility fallback.
std::uint64_t
RandomNoise::seedFromEventHeader(const edm4hep::EventHeaderCollection* headers) const {
  if (headers) {
    return m_uid.getUniqueID(*headers, name());
  }
  if (m_eventHeader) {
    return (static_cast<std::uint64_t>(m_eventHeader->getRunNumber()) << 32) ^
           static_cast<std::uint64_t>(m_eventHeader->getEventNumber()) ^
           std::hash<std::string_view>{}(name());
  }
  warning("RandomNoise '{}': EventHeader not provided; seeding from algorithm name only", name());
  return std::hash<std::string_view>{}(name());
}

// Sample a point uniformly inside the cached sensitive solid, expressed in the
// sensitive component local coordinate frame. Dimensions follow DD4hep conventions.
dd4hep::Position RandomNoise::randomPointInComponent(const SensitiveComponent& component,
                                                     std::mt19937_64& rng) const {
  auto uniform = [&](double lo, double hi) {
    return std::uniform_real_distribution<double>{lo, hi}(rng);
  };

  const auto& d = component.dimensions;
  switch (component.shape) {
  case SensitiveShapeKind::Box:
    return {uniform(-d[0], d[0]), uniform(-d[1], d[1]), uniform(-d[2], d[2])};

  case SensitiveShapeKind::Trd1: {
    const double dx1 = d[0], dx2 = d[1], dy = d[2], dz = d[3];
    const double maxDx = std::max(dx1, dx2);
    // Rejection sampling keeps z uniform over volume when the x half-width changes with z.
    for (;;) {
      const double z  = uniform(-dz, dz);
      const double t  = (z + dz) / (2.0 * dz);
      const double dx = dx1 + t * (dx2 - dx1);
      if (uniform(0.0, maxDx) <= dx) {
        return {uniform(-dx, dx), uniform(-dy, dy), z};
      }
    }
  }

  case SensitiveShapeKind::Trd2: {
    const double dx1 = d[0], dx2 = d[1], dy1 = d[2], dy2 = d[3], dz = d[4];
    const double maxArea = std::max(dx1 * dy1, dx2 * dy2);
    // Rejection sampling weights z by the trapezoid cross-section area.
    for (;;) {
      const double z  = uniform(-dz, dz);
      const double t  = (z + dz) / (2.0 * dz);
      const double dx = dx1 + t * (dx2 - dx1);
      const double dy = dy1 + t * (dy2 - dy1);
      if (uniform(0.0, maxArea) <= dx * dy) {
        return {uniform(-dx, dx), uniform(-dy, dy), z};
      }
    }
  }

  case SensitiveShapeKind::Tube: {
    const double rmin = d[0], rmax = d[1], dz = d[2];
    // Sample r^2 uniformly so hits are uniform in annular area.
    const double r   = std::sqrt(uniform(rmin * rmin, rmax * rmax));
    const double phi = uniform(0.0, 2.0 * M_PI);
    return {r * std::cos(phi), r * std::sin(phi), uniform(-dz, dz)};
  }

  case SensitiveShapeKind::TubeSegment: {
    const double rmin = d[0], rmax = d[1], dz = d[2];
    const double phi1 = d[3];
    double phi2       = d[4];
    if (phi2 < phi1) {
      phi2 += 2.0 * M_PI;
    }
    // DD4hep stores tube segment phi bounds in radians.
    const double r   = std::sqrt(uniform(rmin * rmin, rmax * rmax));
    const double phi = uniform(phi1, phi2);
    return {r * std::cos(phi), r * std::sin(phi), uniform(-dz, dz)};
  }

  case SensitiveShapeKind::Cone: {
    const double dz = d[0], rmin1 = d[1], rmax1 = d[2], rmin2 = d[3], rmax2 = d[4];
    const double maxArea = std::max(rmax1 * rmax1 - rmin1 * rmin1, rmax2 * rmax2 - rmin2 * rmin2);
    // Rejection sampling weights z by the annular area at that z.
    for (;;) {
      const double z    = uniform(-dz, dz);
      const double t    = (z + dz) / (2.0 * dz);
      const double rmin = rmin1 + t * (rmin2 - rmin1);
      const double rmax = rmax1 + t * (rmax2 - rmax1);
      const double area = std::max(0.0, rmax * rmax - rmin * rmin);
      if (uniform(0.0, maxArea) > area) {
        continue;
      }
      const double r   = std::sqrt(uniform(rmin * rmin, rmax * rmax));
      const double phi = uniform(0.0, 2.0 * M_PI);
      return {r * std::cos(phi), r * std::sin(phi), z};
    }
  }

  case SensitiveShapeKind::EllipticalTube: {
    const double a = d[0], b = d[1], dz = d[2];
    // Simple rejection sampling gives a uniform point in the elliptical cross-section.
    for (;;) {
      const double x = uniform(-a, a);
      const double y = uniform(-b, b);
      if ((x * x) / (a * a) + (y * y) / (b * b) <= 1.0) {
        return {x, y, uniform(-dz, dz)};
      }
    }
  }
  }

  throw std::logic_error("unhandled RandomNoise sensitive shape");
}

// Convert one random sensitive-local point to a cell ID:
// pick sensitive component -> full component-local to module-local transform ->
// global position -> DD4hep cell ID.
std::optional<std::uint64_t> RandomNoise::randomCellID(const ModuleGeometry& module,
                                                       std::mt19937_64& rng) const {
  if (module.sensitiveComponents.empty()) {
    return std::nullopt;
  }

  double totalWeight = 0.0;
  for (const auto& component : module.sensitiveComponents) {
    totalWeight += component.samplingWeight;
  }

  const SensitiveComponent* component = &module.sensitiveComponents.front();
  if (totalWeight > 0.0) {
    double pick = std::uniform_real_distribution<double>{0.0, totalWeight}(rng);
    for (const auto& candidate : module.sensitiveComponents) {
      pick -= candidate.samplingWeight;
      if (pick <= 0.0) {
        component = &candidate;
        break;
      }
    }
  } else {
    std::uniform_int_distribution<std::size_t> pickComponent(0,
                                                             module.sensitiveComponents.size() - 1);
    component = &module.sensitiveComponents[pickComponent(rng)];
  }

  const auto componentLocal = randomPointInComponent(*component, rng);
  const auto& rotation      = component->localToModuleRotation;
  const auto& translation   = component->localToModuleTranslation;
  const double local[3]     = {componentLocal.x(), componentLocal.y(), componentLocal.z()};
  const dd4hep::Position moduleLocal{
      rotation[0] * local[0] + rotation[1] * local[1] + rotation[2] * local[2] + translation.x(),
      rotation[3] * local[0] + rotation[4] * local[1] + rotation[5] * local[2] + translation.y(),
      rotation[6] * local[0] + rotation[7] * local[1] + rotation[8] * local[2] + translation.z()};
  const auto global = module.module.nominal().localToWorld(moduleLocal);
  const auto cellID = m_converter->cellID(global);

  if (!m_converter->findContext(cellID)) {
    return std::nullopt;
  }
  return static_cast<std::uint64_t>(cellID);
}

// Draw the layer occupancy from a Poisson distribution, choose modules uniformly,
// and skip duplicate/invalid cell IDs. Very high occupancy can therefore create
// fewer hits than requested.
void RandomNoise::addNoiseHitsForLayer(
    const LayerGeometry& layer, std::map<std::uint64_t, edm4eic::MutableRawTrackerHit>& hitMap,
    std::mt19937_64& rng) const {
  if (layer.modules.empty() || layer.meanNoiseHits <= 0) {
    return;
  }

  std::poisson_distribution<std::size_t> poisson(static_cast<double>(layer.meanNoiseHits));
  const std::size_t requested   = poisson(rng);
  const std::size_t maxAttempts = std::max<std::size_t>(100, requested * 20);
  std::uniform_int_distribution<std::size_t> pickModule(0, layer.modules.size() - 1);

  std::size_t created  = 0;
  std::size_t attempts = 0;
  while (created < requested && attempts < maxAttempts) {
    ++attempts;
    const auto& module = layer.modules[pickModule(rng)];
    auto cellID        = randomCellID(module, rng);
    if (!cellID || hitMap.find(*cellID) != hitMap.end()) {
      continue;
    }

    edm4eic::MutableRawTrackerHit hit;
    hit.setCellID(*cellID);
    hit.setCharge(1.0e6);
    hit.setTimeStamp(0);
    hitMap.emplace(*cellID, hit);
    ++created;
  }

  if (created < requested) {
    warning("RandomNoise '{}': created {}/{} requested hits for detector '{}' layer {} after {} "
            "attempts",
            name(), created, requested, layer.detectorName.empty() ? "<any>" : layer.detectorName,
            layer.layer, attempts);
  }
}

// Event-level entry point: seed the RNG, fill all configured layer groups, and
// emit hits in deterministic cell-ID order.
void RandomNoise::process(const Input& in, const Output& out) const {
  auto [outHits]       = out;
  const auto [headers] = in;

  if (!m_cfg.addNoise) {
    return;
  }
  if (m_layers.empty()) {
    warning("RandomNoise '{}': no configured geometry groups; emitting empty collection", name());
    return;
  }

  std::mt19937_64 rng(seedFromEventHeader(headers));
  std::map<std::uint64_t, edm4eic::MutableRawTrackerHit> noiseHits;

  for (const auto& layer : m_layers) {
    addNoiseHitsForLayer(layer, noiseHits, rng);
  }

  for (const auto& [_, hit] : noiseHits) {
    outHits->push_back(hit);
  }
}

} // namespace eicrecon
