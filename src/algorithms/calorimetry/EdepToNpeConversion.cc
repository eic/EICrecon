// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Minho Kim
//
// Convert the energy deposit of each SimCalorimeterHit into a Poisson-smeared
// number of photoelectrons (Npe). The expected Npe is obtained by multiplying
// the deposited energy by a conversion factor, either constant or read from a
// lookup table keyed on cellID fields (e.g. layer). The smeared Npe is stored
// in the energy field of the output hits, so that downstream algorithms
// (e.g. PulseGeneration) can consume it through the unchanged
// SimCalorimeterHit interface.

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

  info("EdepToNpeConversion> init()");

  if (m_cfg.edep_to_npe < 0) {
    throw std::runtime_error(
        fmt::format("Constant edep-to-npe factor must be non-negative, got {}", m_cfg.edep_to_npe));
  }

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

  info("EdepToNpeConversion> conditions for LUT are all satisfied");

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

  // Load the LUT: each line holds one integer per key field followed by the
  // conversion factor. Empty lines and lines starting with '#' are skipped.
  std::string filename = fmt::format("calibrations/{}", m_cfg.edep_to_npe_filename);
  std::ifstream infile(filename);
  if (!infile) {
    info("EdepToNpeConversion> No LUT file");
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
      info("value = {}", value);
    }
    if (!(iss >> factor)) {
      throw std::runtime_error(fmt::format("Malformed LUT file {} at line {}", filename, lineno));
    }
    info("factor = {}", factor);
    if (!m_edep_to_npe_lut.emplace(std::move(key), factor).second) {
      throw std::runtime_error(
          fmt::format("Duplicate key in LUT file {} at line {}", filename, lineno));
    }
  }
  if (m_edep_to_npe_lut.empty()) {
    throw std::runtime_error(fmt::format("LUT file {} contains no entries", filename));
  }
}

void EdepToNpeConversion::process(const EdepToNpeConversion::Input& input,
                                  const EdepToNpeConversion::Output& output) const {
  const auto [headers, inhits] = input;
  auto [outhits]               = output;

  // Event-local random engine, seeded reproducibly from the event header
  auto seed = m_uid.getUniqueID(*headers, name());
  std::mt19937 generator(seed);

  for (const auto& hit : *inhits) {

    const double mean_npe = hit.getEnergy() * get_edep_to_npe_factor(hit);

    // Photostatistics: the number of detected photoelectrons fluctuates
    // around the expectation following a Poisson distribution
    long npe = 0;
    if (mean_npe > 0) {
      std::poisson_distribution<long> poisson(mean_npe);
      npe = poisson(generator);
    }

    // No photoelectrons detected: no signal to propagate downstream
    if (npe == 0) {
      continue;
    }

    auto out_hit = outhits->create();
    out_hit.setCellID(hit.getCellID());
    // NOTE: the energy field of the output hit carries the (dimensionless)
    // number of photoelectrons, not an energy
    out_hit.setEnergy(static_cast<float>(npe));
    out_hit.setPosition(hit.getPosition());
    // Preserve the contributions: downstream algorithms use them for the hit
    // time and the MC particle relations
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
