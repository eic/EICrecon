// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 EIC-FT

#include "RandomNoise.h"

#include <DD4hep/DetElement.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include <DD4hep/Shapes.h>
#include <DD4hep/Volumes.h>
#include <DDSegmentation/CartesianGridXY.h>
#include <DDSegmentation/CartesianGridXZ.h>
#include <DDSegmentation/CartesianGridXYZ.h>
#include <TGeoNode.h>
#include <fmt/core.h>
#include <fmt/ostream.h>

#include <cstdint>
#include <iostream>
#include <tuple>
#include <utility>

namespace eicrecon {

// Operator to print EncodingInfo for debugging
std::ostream& operator<<(std::ostream& os, const EncodingInfo& info) {
  os << "Identifier: " << info.identifier;
  if (info.id_spec.isValid()) {
    os << ", IDDescriptor: " << info.id_spec.fieldDescription();
  } else {
    os << ", IDDescriptor: [invalid]";
  }
  if (info.segmentation.isValid()) {
    os << ", Segmentation type: " << info.segmentation.type();
  } else {
    os << ", Segmentation: [invalid]";
  }
  return os;
}

void RandomNoise::init(const dd4hep::Detector* detector) { m_dd4hepGeo = detector; }

void RandomNoise::process(const Input& input, const Output& output) const {
  const auto [in_hits] = input;
  auto [out_hits]      = output;

  // If noise is disabled, just clone the input hits to the output and return.
  if (!m_cfg.addNoise) {
    for (const auto& hit : *in_hits) {
      out_hits->push_back(hit.clone());
    }
    return;
  }

  // Create a mutable map of hits from the input collection for efficient modification.
  std::unordered_map<std::uint64_t, edm4eic::MutableRawTrackerHit> cell_hit_map;
  for (const auto& hit : *in_hits) {
    cell_hit_map.emplace(hit.getCellID(), hit.clone());
  }

  // Inject noise hits into the map.
  add_noise_hits(cell_hit_map);

  // Populate the output collection from the final map.
  for (const auto& pair : cell_hit_map) {
    out_hits->push_back(pair.second);
  }
    std::cout << "Collection Sizes: SiBarrelRawHits = " << in_hits->size() 
                           << ", NoisySiBarrelRawHits = " << out_hits->size()<<std::endl; 
  
}

std::size_t RandomNoise::ScanPhysicalVolume(ScanContext& context, dd4hep::DetElement e, dd4hep::PlacedVolume pv) const {
  PlacementPath initial_chain;
  EncodingInfo initial_encoding;
  return ScanPhysicalVolumeRecursive(context, e, pv, initial_encoding, {}, initial_chain);
}

std::size_t RandomNoise::ScanPhysicalVolumeRecursive(ScanContext& context, dd4hep::DetElement e, dd4hep::PlacedVolume pv,
                                                     EncodingInfo parent_encoding, dd4hep::SensitiveDetector sd,
                                                     PlacementPath& chain) const {
  TGeoNode* node    = pv.ptr();
  std::size_t count = 0;
  if (node) {
    dd4hep::Volume vol                                  = pv.volume();
    const dd4hep::PlacedVolumeExtension::VolIDs& pv_ids = pv.volIDs();
    EncodingInfo vol_encoding                           = parent_encoding;
    bool is_sensitive                                   = vol.isSensitive();

    if (!sd.isValid()) {
      dd4hep::SensitiveDetector de_sd = context.detector.sensitiveDetector(e.name());
      if (de_sd.isValid()) {
        sd = de_sd;
      }
    }
    if (is_sensitive) {
      sd = vol.sensitiveDetector();
    }

    chain.emplace_back(node);

    if (sd.isValid() && !pv_ids.empty()) {
      dd4hep::Readout ro = sd.readout();
      if (ro.isValid()) {
        const dd4hep::IDDescriptor& id_spec = ro.idSpec();
        vol_encoding.identifier |= id_spec.encode(pv_ids);
        vol_encoding.id_spec      = id_spec;
        vol_encoding.segmentation = ro.segmentation();
      }
    }

    for (int idau = 0, ndau = node->GetNdaughters(); idau < ndau; ++idau) {
      TGeoNode* daughter_node = node->GetDaughter(idau);
      dd4hep::PlacedVolume place_dau(daughter_node);
      if (place_dau.isValid()) {
        dd4hep::DetElement de_dau;
        for (const auto& child_de_pair : e.children()) {
          if (child_de_pair.second.placement().ptr() == daughter_node) {
            de_dau = child_de_pair.second;
            break;
          }
        }
        if (de_dau.isValid()) {
          PlacementPath dau_chain;
          count += ScanPhysicalVolumeRecursive(context, de_dau, place_dau, vol_encoding, sd, dau_chain);
        } else {
          count += ScanPhysicalVolumeRecursive(context, e, place_dau, vol_encoding, sd, chain);
        }
      }
    }

    if (is_sensitive) {
      context.results[e].push_back(vol_encoding);
      context.node_count++;
      count++;
    }
    chain.pop_back();
  }
  return count;
}

RandomNoise::SensorNoiseInfoVec RandomNoise::GenericGridScanner(
    [[maybe_unused]] const std::vector<std::string>& fields, dd4hep::PlacedVolume pv, const EncodingInfo& encoding) const {
  SensorNoiseInfoVec sensor_infos;
  dd4hep::Segmentation seg = encoding.segmentation;
  dd4hep::Solid solid      = pv.volume().solid();
  dd4hep::Box box(solid);
  if (!box.isValid()) {
    fmt::print(stderr, "INFO: Sensor shape for '{}' is not a Box, cannot determine grid boundaries.\n", pv.name());
    return sensor_infos;
  }

  long num_x = 0, num_y = 0, num_z = 0;

  if (auto grid = dynamic_cast<dd4hep::DDSegmentation::CartesianGridXY*>(seg.segmentation())) {
    num_x = static_cast<long>(2 * box.x() / grid->gridSizeX());
    num_y = static_cast<long>(2 * box.y() / grid->gridSizeY());
  } else if (auto grid = dynamic_cast<dd4hep::DDSegmentation::CartesianGridXZ*>(seg.segmentation())) {
    num_x = static_cast<long>(2 * box.x() / grid->gridSizeX());
    num_z = static_cast<long>(2 * box.z() / grid->gridSizeZ());
  } else if (auto grid = dynamic_cast<dd4hep::DDSegmentation::CartesianGridXYZ*>(seg.segmentation())) {
    num_x = static_cast<long>(2 * box.x() / grid->gridSizeX());
    num_y = static_cast<long>(2 * box.y() / grid->gridSizeY());
    num_z = static_cast<long>(2 * box.z() / grid->gridSizeZ());
  }

  sensor_infos.push_back({encoding, std::make_tuple(num_x, num_y, num_z)});
  return sensor_infos;
}

RandomNoise::SensorNoiseInfoVec RandomNoise::ScanSensorCells(dd4hep::PlacedVolume pv, const EncodingInfo& encoding) const {
  if (!encoding.segmentation.isValid()) return {};

  dd4hep::Segmentation seg = encoding.segmentation;
  std::string type         = seg.type();
  if (type == "CartesianGridXY") {
    return GenericGridScanner({"x", "y"}, pv, encoding);
  } else if (type == "CartesianGridXZ") {
    return GenericGridScanner({"x", "z"}, pv, encoding);
  } else if (type == "CartesianGridXYZ") {
    return GenericGridScanner({"x", "y", "z"}, pv, encoding);
  }

  fmt::print("    INFO: This tool does not currently scan details for segmentation type '{}'.\n", type);
  return {};
}

void RandomNoise::injectNoise(const SensorNoiseInfoVec& sensor_infos,
                              std::unordered_map<std::uint64_t, edm4eic::MutableRawTrackerHit>& cell_hit_map,
                              int nNoiseHits) const {
  if (sensor_infos.empty() || nNoiseHits == 0) return;

  if (sensor_infos.size() > 1) {
    const auto& first_dimensions = sensor_infos.front().dimensions;
    bool all_dimensions_are_the_same =
        std::all_of(sensor_infos.begin() + 1, sensor_infos.end(),
                    [&](const auto& info) { return info.dimensions == first_dimensions; });
    if (!all_dimensions_are_the_same) {
      fmt::print(stderr, "WARNING: Sensor dimensions are NOT uniform. Noise injection for this group is skipped.\n");
      return;
    }
  }

  for (int i = 0; i < nNoiseHits; ++i) {
    const auto& random_sensor_info = sensor_infos[m_random.Integer(sensor_infos.size())];
    const auto& encoding           = random_sensor_info.encoding;
    auto [nx, ny, nz]              = random_sensor_info.dimensions;

    auto decoder                   = encoding.id_spec.decoder();
    dd4hep::VolumeID noise_cell_id = encoding.identifier;

    long rand_ix = (nx > 0) ? (m_random.Integer(nx) - nx / 2) : 0;
    long rand_iy = (ny > 0) ? (m_random.Integer(ny) - ny / 2) : 0;
    long rand_iz = (nz > 0) ? (m_random.Integer(nz) - nz / 2) : 0;

    if (nx > 0) decoder->set(noise_cell_id, "x", rand_ix);
    if (ny > 0) decoder->set(noise_cell_id, "y", rand_iy);
    if (nz > 0) decoder->set(noise_cell_id, "z", rand_iz);

    if (cell_hit_map.find(noise_cell_id) == cell_hit_map.end()) {
      cell_hit_map.emplace(noise_cell_id, edm4eic::MutableRawTrackerHit{noise_cell_id, 1, 0});
    } else {
      i--; // Retry if the cell is already occupied.
    }
  }
}

void RandomNoise::add_noise_hits(std::unordered_map<std::uint64_t, edm4eic::MutableRawTrackerHit>& cell_hit_map) const {
  if (!m_cfg.addNoise || m_cfg.target_layers.empty() || !m_dd4hepGeo) {
    return;
  }

  for (const auto& target_layer : m_cfg.target_layers) {
    SensorNoiseInfoVec current_system_infos;
    const auto& region      = target_layer.first;
    const auto& layer_index = target_layer.second;
    const char* system_name = nullptr;
    switch (region) {
      case SubdetectorRegion::barrel:
        if (layer_index > 0 && layer_index < 4) system_name = "VertexBarrel";
        else if (layer_index == 4) system_name = "SagittaSiBarrel";
        else if (layer_index == 5) system_name = "OuterSiBarrel";
        break;
      case SubdetectorRegion::backward:
        if (layer_index == 1) system_name = "InnerTrackerEndcapN";
        else if (layer_index == 2) system_name = "MiddleTrackerEndcapN";
        else if (layer_index == 3) system_name = "OuterTrackerEndcapN";
        break;
      case SubdetectorRegion::forward:
        if (layer_index == 1) system_name = "InnerTrackerEndcapP";
        else if (layer_index == 2) system_name = "MiddleTrackerEndcapP";
        else if (layer_index == 3) system_name = "OuterTrackerEndcapP";
        break;
    }

    if (!system_name) {
      fmt::print(stderr, "WARNING: Skipping invalid noise configuration for layer {}\n", layer_index);
      continue;
    }

    auto start_element = m_dd4hepGeo->detector(system_name);
    ScanContext context(*m_dd4hepGeo);
    ScanPhysicalVolume(context, start_element, start_element.placement());

    if (context.results.empty()) {
      fmt::print(stderr, "WARNING: No sensitive components found to scan in system '{}'.\n", system_name);
      continue;
    }

    for (const auto& pair : context.results) {
      dd4hep::DetElement de = pair.first;
      if (!pair.second.empty()) {
        auto infos = ScanSensorCells(de.placement(), pair.second.front());
        current_system_infos.insert(current_system_infos.end(), infos.begin(), infos.end());
      }
    }

    if (!current_system_infos.empty()) {
      long n_hits_for_this_system = m_random.Poisson(m_cfg.n_noise_hits_per_system);
      injectNoise(current_system_infos, cell_hit_map, n_hits_for_this_system);
    }
  }
}

} // namespace eicrecon