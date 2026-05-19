// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Minjung Kim, Joshua Sobaljic, Shujie Li

#pragma once

#include <DD4hep/DetElement.h>
#include <DD4hep/Detector.h>
#include <DD4hep/Objects.h>
#include <DD4hep/Readout.h>
#include <DDRec/CellIDPositionConverter.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4hep/EventHeaderCollection.h>
#include <array>
#include <cstdint>
#include <map>
#include <optional>
#include <random>
#include <string>
#include <string_view>
#include <vector>

#include "RandomNoiseConfig.h"
#include "algorithms/algorithm.h"
#include "algorithms/interfaces/UniqueIDGenSvc.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using RandomNoiseAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4hep::EventHeaderCollection>,
                          algorithms::Output<edm4eic::RawTrackerHitCollection>>;

class RandomNoise : public RandomNoiseAlgorithm, public WithPodConfig<RandomNoiseConfig> {
public:
  explicit RandomNoise(std::string_view name)
      : RandomNoiseAlgorithm{name,
                             {"EventHeader"},
                             {"outputRawHitCollection"},
                             "Generates noise-only RawTrackerHits for a given readout."} {}

  void init();
  void process(const Input&, const Output&) const final;

  // Optional fallback for callers that cannot provide an EventHeader collection.
  void setEventHeader(const edm4hep::EventHeader* evtHeader) { m_eventHeader = evtHeader; }

  enum class SensitiveShapeKind {
    Box,
    Trd1,
    Trd2,
    Tube,
    TubeSegment,
    Cone,
    EllipticalTube,
  };

  struct SensitiveComponent {
    SensitiveShapeKind shape = SensitiveShapeKind::Box;
    std::vector<double> dimensions;
    std::array<double, 9> localToModuleRotation{1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
    dd4hep::Position localToModuleTranslation{0.0, 0.0, 0.0};
    double samplingWeight = 1.0;
    std::string shapeName;
  };

  struct ModuleGeometry {
    std::string detectorName;
    int layer = 0;
    dd4hep::DetElement module;
    std::vector<SensitiveComponent> sensitiveComponents;
  };

  struct LayerGeometry {
    std::string detectorName;
    int layer         = 0;
    int meanNoiseHits = 0;
    std::vector<ModuleGeometry> modules;
  };

private:
  void collectDetectorModules(const dd4hep::DetElement& detector);
  void collectLayerModules(const std::string& detectorName, const dd4hep::DetElement& layer);
  void buildConfiguredLayers();

  std::uint64_t seedFromEventHeader(const edm4hep::EventHeaderCollection* headers) const;
  dd4hep::Position randomPointInComponent(const SensitiveComponent& component,
                                          std::mt19937_64& rng) const;
  std::optional<std::uint64_t> randomCellID(const ModuleGeometry& module,
                                            std::mt19937_64& rng) const;
  void addNoiseHitsForLayer(const LayerGeometry& layer,
                            std::map<std::uint64_t, edm4eic::MutableRawTrackerHit>& hitMap,
                            std::mt19937_64& rng) const;

  const edm4hep::EventHeader* m_eventHeader = nullptr;
  dd4hep::Readout m_readout;
  std::vector<ModuleGeometry> m_modules;
  std::vector<LayerGeometry> m_layers;
  const dd4hep::Detector* m_dd4hepGeo                     = nullptr;
  const dd4hep::rec::CellIDPositionConverter* m_converter = nullptr;
  const algorithms::UniqueIDGenSvc& m_uid                 = algorithms::UniqueIDGenSvc::instance();
};

} // namespace eicrecon
