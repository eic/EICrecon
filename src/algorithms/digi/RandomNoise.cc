// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minjung Kim, Joshua Sobaljic, Shujie Li
//
// RandomNoise generates noise-only RawTrackerHits by caching DD4hep module and
// sensitive-shape geometry once, then sampling random positions per configured
// detector layer and converting those positions to valid tracker cell IDs.

#include "RandomNoise.h"

#include <DD4hep/Alignments.h>
#include <DD4hep/Volumes.h>
#include <TGeoBBox.h>
#include <TGeoMatrix.h>
#include <TGeoNode.h>
#include <TGeoShape.h>
#include <TGeoVolume.h>
#include <algorithms/geo.h>
#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <gsl/pointers>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <utility>

#include "algorithms/digi/RandomNoiseConfig.h"

namespace eicrecon {
namespace {

  struct LocalTransform {
    std::array<double, 9> rotation{1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
    dd4hep::Position translation{0.0, 0.0, 0.0};
  };

  // Normalize DD4hep field names before matching layer-like volume IDs.
  std::string lower(std::string s) {
    for (auto& c : s) {
      c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return s;
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

  // Resolve the layer ID only from explicit DD4hep volume IDs; missing layer IDs
  // are treated as geometry/configuration errors.
  std::optional<int> layerFromDetElement(const dd4hep::DetElement& layer,
                                         const dd4hep::DetElement& module) {
    if (auto id = layerFromPlacement(layer.placement())) {
      return id;
    }
    if (auto id = layerFromPlacement(module.placement())) {
      return id;
    }
    return std::nullopt;
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

  // ROOT shapes commonly inherit TGeoBBox, which exposes the local bounds used
  // for generic rejection sampling before the final TGeo Contains() check.
  bool cacheLocalBounds(const TGeoShape& shape, RandomNoise::SensitiveComponent& component) {
    const auto* box = dynamic_cast<const TGeoBBox*>(&shape);
    if (!box) {
      return false;
    }

    const auto* origin         = box->GetOrigin();
    component.boundsCenter     = {origin[0], origin[1], origin[2]};
    component.boundsHalfLength = {box->GetDX(), box->GetDY(), box->GetDZ()};
    return component.boundsHalfLength[0] > 0.0 && component.boundsHalfLength[1] > 0.0 &&
           component.boundsHalfLength[2] > 0.0;
  }

  // Cache the TGeo sensitive volume once. Sampling later uses our event RNG for
  // reproducibility, while TGeo Capacity()/Contains() own the shape semantics.
  std::optional<RandomNoise::SensitiveComponent>
  componentFromPlacedVolume(const dd4hep::PlacedVolume& pv, const LocalTransform& localToModule) {
    if (!pv.isValid()) {
      return std::nullopt;
    }

    RandomNoise::SensitiveComponent component;
    component.localToModuleRotation    = localToModule.rotation;
    component.localToModuleTranslation = localToModule.translation;
    auto volume                        = pv.volume();
    component.volume                   = volume.ptr();
    if (!component.volume || !component.volume->GetShape()) {
      throw std::runtime_error("RandomNoise sensitive volume has no TGeo shape");
    }

    const auto* shape   = component.volume->GetShape();
    component.shapeName = shape->ClassName();
    if (!cacheLocalBounds(*shape, component)) {
      throw std::runtime_error("RandomNoise cannot cache local TGeo bounds for sensitive volume");
    }
    component.samplingWeight = std::max(component.volume->Capacity(), 0.0);
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

  // Sensitive components are the sampling units. Equal shape/capacity is the
  // equal-area check used before sampling those components uniformly.
  bool sameSensitiveComponent(const RandomNoise::SensitiveComponent& lhs,
                              const RandomNoise::SensitiveComponent& rhs) {
    constexpr double tolerance = 1e-9;
    if (lhs.shapeName != rhs.shapeName) {
      return false;
    }
    return std::abs(lhs.samplingWeight - rhs.samplingWeight) <= tolerance;
  }

  // Flatten each layer to component targets while retaining the parent module
  // needed for the local-to-global transform and cell-ID conversion.
  void buildSensitiveTargets(RandomNoise::LayerGeometry& layer) {
    layer.sensitiveTargets.clear();
    for (std::size_t moduleIndex = 0; moduleIndex < layer.modules.size(); ++moduleIndex) {
      const auto& module = layer.modules[moduleIndex];
      for (std::size_t componentIndex = 0; componentIndex < module.sensitiveComponents.size();
           ++componentIndex) {
        layer.sensitiveTargets.push_back({moduleIndex, componentIndex});
      }
    }
  }

  const RandomNoise::SensitiveComponent&
  componentForTarget(const RandomNoise::LayerGeometry& layer,
                     const RandomNoise::SensitiveTarget& target) {
    return layer.modules[target.moduleIndex].sensitiveComponents[target.componentIndex];
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
    if (!components.empty()) {
      if (!layerID) {
        error("RandomNoise '{}': detector '{}' layer '{}' module '{}' has sensitive components but "
              "no explicit DD4hep layer volID",
              name(), detectorName, layer.name(), module.name());
        throw std::runtime_error("RandomNoise requires explicit DD4hep layer volIDs");
      }
      m_modules.push_back({detectorName, *layerID, module, std::move(components)});
      continue;
    }

    for (const auto& [__, child] : module.children()) {
      auto childLayerID    = layerID ? layerID : layerFromDetElement(layer, child);
      auto childComponents = findSensitiveComponents(child, m_cfg.readout_name);
      if (childComponents.empty()) {
        continue;
      }
      if (!childLayerID) {
        error("RandomNoise '{}': detector '{}' layer '{}' module '{}' child '{}' has sensitive "
              "components but no explicit DD4hep layer volID",
              name(), detectorName, layer.name(), module.name(), child.name());
        throw std::runtime_error("RandomNoise requires explicit DD4hep layer volIDs");
      }
      m_modules.push_back({detectorName, *childLayerID, child, std::move(childComponents)});
    }
  }
}

// Convert the configured per-layer means into cached layer groups. Noise is
// sampled uniformly over sensitive components, so those components should have
// equal active area within each configured group.
void RandomNoise::buildConfiguredLayers() {
  const bool hasLayerConfig = !m_cfg.layer_id.empty() && !m_cfg.n_noise_hits_per_layer.empty() &&
                              m_cfg.layer_id.size() == m_cfg.n_noise_hits_per_layer.size();

  if (!hasLayerConfig) {
    if (!m_modules.empty()) {
      LayerGeometry layer{"", -1, m_cfg.n_noise_hits_per_system, m_modules, {}};
      buildSensitiveTargets(layer);
      m_layers.push_back(std::move(layer));
      warning("RandomNoise '{}': using system-wide noise mean {} because layer configuration is "
              "missing or inconsistent",
              name(), m_cfg.n_noise_hits_per_system);
    }
    return;
  }

  for (std::size_t i = 0; i < m_cfg.layer_id.size(); ++i) {
    const std::string detectorFilter =
        i < m_cfg.detector_names.size() ? m_cfg.detector_names[i] : std::string{};

    LayerGeometry layer{detectorFilter, m_cfg.layer_id[i], m_cfg.n_noise_hits_per_layer[i], {}, {}};
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

    buildSensitiveTargets(layer);
    if (layer.sensitiveTargets.empty()) {
      warning("RandomNoise '{}': no sensitive components found for detector '{}' layer {}", name(),
              detectorFilter.empty() ? "<any>" : detectorFilter, layer.layer);
      continue;
    }

    const auto& firstComponent = componentForTarget(layer, layer.sensitiveTargets.front());
    debug("RandomNoise geometry cache: detector='{}' layer={} modules={} "
          "sensitive_components={} first_shape='{}' first_capacity={} "
          "first_bounds_half_lengths=[{}, {}, {}]",
          detectorFilter.empty() ? layer.modules.front().detectorName : detectorFilter, layer.layer,
          layer.modules.size(), layer.sensitiveTargets.size(), firstComponent.shapeName,
          firstComponent.samplingWeight, firstComponent.boundsHalfLength[0],
          firstComponent.boundsHalfLength[1], firstComponent.boundsHalfLength[2]);
    for (const auto& target : layer.sensitiveTargets) {
      if (!sameSensitiveComponent(firstComponent, componentForTarget(layer, target))) {
        warning("RandomNoise '{}': detector '{}' layer {} contains non-identical sensitive "
                "components; uniform component sampling assumes equal active area",
                name(),
                detectorFilter.empty() ? layer.modules.front().detectorName : detectorFilter,
                layer.layer);
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

// Sample in sensitive-local coordinates with our event RNG, using TGeo
// Contains() as the source of truth for non-box shapes.
dd4hep::Position RandomNoise::randomPointInComponent(const SensitiveComponent& component,
                                                     std::mt19937_64& rng) const {
  if (!component.volume) {
    throw std::runtime_error("RandomNoise sensitive component has no TGeo volume");
  }

  auto uniform = [&](double lo, double hi) {
    return std::uniform_real_distribution<double>{lo, hi}(rng);
  };

  constexpr std::size_t maxAttempts = 100000;
  for (std::size_t attempt = 0; attempt < maxAttempts; ++attempt) {
    double local[3] = {
        uniform(component.boundsCenter[0] - component.boundsHalfLength[0],
                component.boundsCenter[0] + component.boundsHalfLength[0]),
        uniform(component.boundsCenter[1] - component.boundsHalfLength[1],
                component.boundsCenter[1] + component.boundsHalfLength[1]),
        uniform(component.boundsCenter[2] - component.boundsHalfLength[2],
                component.boundsCenter[2] + component.boundsHalfLength[2]),
    };

    if (component.volume->Contains(local)) {
      return {local[0], local[1], local[2]};
    }
  }

  throw std::runtime_error("RandomNoise failed to sample inside TGeo sensitive volume");
}

// Convert one random sensitive-local point to a cell ID:
// component-local point -> module-local transform -> global position -> DD4hep cell ID.
std::optional<std::uint64_t> RandomNoise::randomCellID(const ModuleGeometry& module,
                                                       const SensitiveComponent& component,
                                                       std::mt19937_64& rng) const {
  const auto componentLocal = randomPointInComponent(component, rng);
  const auto& rotation      = component.localToModuleRotation;
  const auto& translation   = component.localToModuleTranslation;
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

// Draw the layer occupancy from a Poisson distribution, choose sensitive
// components uniformly, and skip duplicate/invalid cell IDs. Very high occupancy
// can therefore create fewer hits than requested.
void RandomNoise::addNoiseHitsForLayer(
    const LayerGeometry& layer, std::map<std::uint64_t, edm4eic::MutableRawTrackerHit>& hitMap,
    std::mt19937_64& rng) const {
  if (layer.sensitiveTargets.empty() || layer.meanNoiseHits <= 0) {
    return;
  }

  std::poisson_distribution<std::size_t> poisson(static_cast<double>(layer.meanNoiseHits));
  const std::size_t requested   = poisson(rng);
  const std::size_t maxAttempts = std::max<std::size_t>(100, requested * 20);
  std::uniform_int_distribution<std::size_t> pickTarget(0, layer.sensitiveTargets.size() - 1);

  std::size_t created  = 0;
  std::size_t attempts = 0;
  while (created < requested && attempts < maxAttempts) {
    ++attempts;
    const auto& target    = layer.sensitiveTargets[pickTarget(rng)];
    const auto& module    = layer.modules[target.moduleIndex];
    const auto& component = module.sensitiveComponents[target.componentIndex];
    auto cellID           = randomCellID(module, component, rng);
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
