// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Minho Kim

#include <DD4hep/Detector.h>
#include <DD4hep/Readout.h>
#include <edm4hep/CaloHitContribution.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <podio/RelationRange.h>
#include <cstddef>
#include <fstream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "EdepToNpeConversion.h"

namespace eicrecon {

void EdepToNpeConversion::init() {

  // A nonzero constant factor takes precedence if it is configured together with LUT
  if (m_cfg.edep_to_npe > 0) {
    if (!m_cfg.edep_to_npe_filename.empty()) {
      info("Constant edep-to-npe factor {} is set; ignoring LUT file {}", m_cfg.edep_to_npe,
           m_cfg.edep_to_npe_filename);
    }
    return;
  }

  // Otherwise a complete LUT configuration is required
  if (m_cfg.readout.empty() || m_cfg.edep_to_npe_fields.empty() ||
      m_cfg.edep_to_npe_filename.empty()) {
    throw std::runtime_error(
        "No edep-to-npe conversion configured: set either a constant edep_to_npe factor or all "
        "of (readout, edep_to_npe_fields, edep_to_npe_filename)");
  }

  // Get the cellID decoder and the indices of the LUT key fields
  try {
    m_id_spec = m_detector->readout(m_cfg.readout).idSpec();
  } catch (...) {
    throw std::runtime_error(fmt::format("Failed to get idSpec for readout {}", m_cfg.readout));
  }
  m_id_dec = m_id_spec.decoder();
  if (m_id_dec == nullptr) {
    throw std::runtime_error(fmt::format("Failed to get ID decoder for readout {}", m_cfg.readout));
  }
  for (const auto& field : m_cfg.edep_to_npe_fields) {
    try {
      m_field_idxs.push_back(m_id_dec->index(field));
    } catch (...) {
      throw std::runtime_error(
          fmt::format("Field {} not found in idSpec of readout {}", field, m_cfg.readout));
    }
  }

  // Load the LUT and parses each line into a lookup key and a conversion factor
  std::string filename = fmt::format("calibrations/{}", m_cfg.edep_to_npe_filename);
  std::ifstream infile(filename);
  if (!infile) {
    throw std::runtime_error(fmt::format("Unable to open LUT file: {}", filename));
  }
  std::string line;
  std::size_t lineno = 0;
  while (std::getline(infile, line)) {
    lineno++;
    if (line.empty()) {
      throw std::runtime_error(
          fmt::format("Empty line in LUT file {} at line {}", filename, lineno));
    }
    std::istringstream iss(line);
    std::vector<int> key(m_cfg.edep_to_npe_fields.size());
    double factor;
    for (auto& value : key) {
      if (!(iss >> value)) {
        throw std::runtime_error(fmt::format("Malformed LUT file {} at line {}", filename, lineno));
      }
    }
    if (!(iss >> factor)) {
      throw std::runtime_error(fmt::format("Malformed LUT file {} at line {}", filename, lineno));
    }
    if (!m_edep_to_npe_lut.emplace(std::move(key), factor).second) {
      throw std::runtime_error(
          fmt::format("Duplicate key in LUT file {} at line {}", filename, lineno));
    }
  }
  if (m_edep_to_npe_lut.empty()) {
    throw std::runtime_error(fmt::format("LUT file {} contains no entries", filename));
  }
} // EdepToNpeConversion:init

void EdepToNpeConversion::process(const EdepToNpeConversion::Input& input,
                                  const EdepToNpeConversion::Output& output) const {
  const auto [headers, inhits] = input;
  auto [outhits]               = output;

  auto seed = m_uid.getUniqueID(*headers, name());
  std::mt19937 generator(seed);

  for (const auto& hit : *inhits) {
    // Edep-to-Npe conversion & Apply Poisson smearing
    const double mean_npe = hit.getEnergy() * get_edep_to_npe_factor(hit);
    long npe              = 0;
    if (mean_npe > 0) {
      std::poisson_distribution<long> poisson(mean_npe);
      npe = poisson(generator);
    }

    if (npe == 0) {
      continue;
    }

    auto out_hit = outhits->create();
    out_hit.setCellID(hit.getCellID());
    out_hit.setEnergy(static_cast<float>(npe));
    out_hit.setPosition(hit.getPosition());
    for (const auto& contrib : hit.getContributions()) {
      out_hit.addToContributions(contrib);
    }
  }

} // EdepToNpeConversion:process

double EdepToNpeConversion::get_edep_to_npe_factor(const edm4hep::SimCalorimeterHit& hit) const {
  if (m_cfg.edep_to_npe > 0) {
    return m_cfg.edep_to_npe;
  }
  std::vector<int> key;
  key.reserve(m_field_idxs.size());
  for (const auto idx : m_field_idxs) {
    key.push_back(static_cast<int>(m_id_dec->get(hit.getCellID(), idx)));
  }
  const auto it = m_edep_to_npe_lut.find(key);
  if (it == m_edep_to_npe_lut.end()) {
    throw std::runtime_error(
        fmt::format("No edep-to-npe factor for cellID {:#x} (fields [{}] = [{}])", hit.getCellID(),
                    fmt::join(m_cfg.edep_to_npe_fields, ", "), fmt::join(key, ", ")));
  }
  return it->second;
}

} // namespace eicrecon
