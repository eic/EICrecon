// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck, Sylvester Joosten, Dmitry Romanov

#pragma once

#include <TRandom3.h>
#include <DD4hep/DetElement.h>
#include <DD4hep/Detector.h>
#include <DD4hep/Volumes.h>
#include <DDRec/CellIDPositionConverter.h> 
#include <DDSegmentation/Segmentation.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>

#include <functional>
#include <map>
#include <string>
#include <string_view>
#include <vector>
#include <tuple> 

#include "SiliconTrackerDigiConfig.h"
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

// New struct to hold all info needed for noise generation for a sensor
struct SensorNoiseInfo {
    EncodingInfo encoding;
    std::tuple<long, long, long> dimensions;
};

// Define Algorithm type alias
using SiliconTrackerDigiAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4hep::SimTrackerHitCollection>,
                          algorithms::Output<edm4eic::RawTrackerHitCollection,
                                             edm4eic::MCRecoTrackerHitAssociationCollection>>;

class SiliconTrackerDigi : public SiliconTrackerDigiAlgorithm,
                           public WithPodConfig<SiliconTrackerDigiConfig> {

public:
    SiliconTrackerDigi(std::string_view name)
        : SiliconTrackerDigiAlgorithm{name,
                                      {"inputHitCollection"},
                                      {"outputRawHitCollection", "outputHitAssociations"},
                                      "Apply threshold, digitize within ADC range, "
                                      "convert time with smearing resolution."} {}

    void init(const dd4hep::Detector* detector,
              const dd4hep::rec::CellIDPositionConverter* converter);

    void process(const Input&, const Output&) const final;

private:
    using SensorNoiseInfoVec = std::vector<SensorNoiseInfo>;

    /** Random number generation */
    mutable TRandom3 m_random{0};
    std::function<double()> m_gauss;

    const dd4hep::Detector* m_dd4hepGeo = nullptr;
    const dd4hep::rec::CellIDPositionConverter* m_converter = nullptr;

    // --- Noise and Geometry Scanning Member Functions ---
    void add_noise_hits(std::unordered_map<std::uint64_t, edm4eic::MutableRawTrackerHit>& cell_hit_map) const;
    
    // Updated placeholder for the actual noise injection logic
    void injectNoise(const SensorNoiseInfoVec& sensor_infos, 
                     std::unordered_map<std::uint64_t, edm4eic::MutableRawTrackerHit>& cell_hit_map,
                     int nNoiseHits) const;

    std::size_t ScanPhysicalVolume(ScanContext& context, dd4hep::DetElement e, dd4hep::PlacedVolume pv) const;

    std::size_t ScanPhysicalVolumeRecursive(ScanContext& context, dd4hep::DetElement e, dd4hep::PlacedVolume pv,
                                            EncodingInfo parent_encoding, dd4hep::SensitiveDetector sd,
                                            PlacementPath& chain) const;

    SensorNoiseInfoVec ScanSensorCells(dd4hep::PlacedVolume pv, const EncodingInfo& encoding) const;
    
    SensorNoiseInfoVec GenericGridScanner(const std::vector<std::string>& fields, dd4hep::PlacedVolume pv,
                                                    const EncodingInfo& encoding) const;
};

} // namespace eicrecon
