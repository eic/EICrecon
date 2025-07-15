// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 EIC-FT

#pragma once

#include <TRandom3.h>
#include <DD4hep/DetElement.h>
#include <DD4hep/Detector.h>
#include <DD4hep/Volumes.h>
#include <DDSegmentation/Segmentation.h>
#include <edm4eic/RawTrackerHitCollection.h>

#include <functional>
#include <map>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "RandomNoiseConfig.h"
#include "algorithms/algorithm.h"
#include "algorithms/interfaces/WithPodConfig.h"

// Forward declaration for TGeoNode
class TGeoNode;

namespace eicrecon {

// --- Helper types and context structs for geometry scanning ---
using PlacementPath = std::vector<TGeoNode*>;

struct EncodingInfo {
  dd4hep::VolumeID identifier = 0;
  dd4hep::IDDescriptor id_spec;
  dd4hep::Segmentation segmentation;
};

struct ScanContext {
  const dd4hep::Detector& detector;
  std::map<dd4hep::DetElement, std::vector<EncodingInfo>> results;
  std::size_t node_count = 0;
  ScanContext(const dd4hep::Detector& det) : detector(det) {}
};

// Struct to hold all info needed for noise generation for a sensor
struct SensorNoiseInfo {
  EncodingInfo encoding;
  std::tuple<long, long, long> dimensions;
};

// Define Algorithm type alias
using RandomNoiseAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::RawTrackerHitCollection>,
                          algorithms::Output<edm4eic::RawTrackerHitCollection>>;

class RandomNoise : public RandomNoiseAlgorithm, public WithPodConfig<RandomNoiseConfig> {

public:
  RandomNoise(std::string_view name)
      : RandomNoiseAlgorithm{name,
                             {"inputRawHitCollection"},
                             {"outputRawHitCollection"},
                             "Injects random noise hits into a RawTrackerHitCollection."} {}

  void init(const dd4hep::Detector* detector);
  void process(const Input&, const Output&) const final;

private:
  using SensorNoiseInfoVec = std::vector<SensorNoiseInfo>;

  mutable TRandom3 m_random{0};
  const dd4hep::Detector* m_dd4hepGeo = nullptr;

  void add_noise_hits(std::unordered_map<std::uint64_t, edm4eic::MutableRawTrackerHit>& cell_hit_map) const;
  void injectNoise(const SensorNoiseInfoVec& sensor_infos,
                   std::unordered_map<std::uint64_t, edm4eic::MutableRawTrackerHit>& cell_hit_map, int nNoiseHits) const;
  std::size_t ScanPhysicalVolume(ScanContext& context, dd4hep::DetElement e, dd4hep::PlacedVolume pv) const;
  std::size_t ScanPhysicalVolumeRecursive(ScanContext& context, dd4hep::DetElement e, dd4hep::PlacedVolume pv,
                                          EncodingInfo parent_encoding, dd4hep::SensitiveDetector sd, PlacementPath& chain) const;
  SensorNoiseInfoVec ScanSensorCells(dd4hep::PlacedVolume pv, const EncodingInfo& encoding) const;
  SensorNoiseInfoVec GenericGridScanner(const std::vector<std::string>& fields, dd4hep::PlacedVolume pv,
                                        const EncodingInfo& encoding) const;
};

} // namespace eicrecon