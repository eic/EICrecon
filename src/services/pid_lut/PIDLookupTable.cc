// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Nathan Brei, Dmitry Kalinkin

#include "PIDLookupTable.h"
#include <algorithm>
#include <iostream>
#include <fstream> // IWYU pragma: keep
#include <sstream> // IWYU pragma: keep
#include <iterator>
#include <stdexcept>
#include <utility>

namespace bh = boost::histogram;

namespace eicrecon {

const PIDLookupTable::Entry* PIDLookupTable::Lookup(int pdg, int charge, double momentum, double eta_deg, double phi_deg) const {

    if (pdg < 0) {
        pdg *= -1;
        // Our lookup table expects _unsigned_ PDGs. The charge information is passed separately.
    }

    auto pdg_bin = FindBin(m_pdg_binning, pdg);
    if (!pdg_bin.has_value()) return nullptr;

    auto charge_bin = FindBin(m_charge_binning, charge);
    if (!charge_bin.has_value()) return nullptr;

    auto momentum_bin = FindBin(m_momentum_binning, momentum);
    if (!momentum_bin.has_value()) return nullptr;

    auto eta_bin = FindBin(m_eta_binning, eta_deg);
    if (!eta_bin.has_value()) return nullptr;

    auto phi_bin = FindBin(m_phi_binning, phi_deg);
    if (!phi_bin.has_value()) return nullptr;

    size_t offset = 1;
    size_t index = *phi_bin * offset;

    offset *= m_phi_binning.bin_count;
    index += *eta_bin * offset;

    offset *= m_eta_binning.bin_count;
    index += *momentum_bin * offset;

    offset *= m_momentum_binning.bin_count;
    index += *charge_bin * offset;

    offset *= m_charge_binning.size();
    index += *pdg_bin * offset;

    auto entry_table = m_table.at(index);
    auto entry_hist = m_hist[
      decltype(m_hist)::multi_index_type {
        m_hist.axis(0).index(pdg),
        m_hist.axis(1).index(charge),
        m_hist.axis(2).index(momentum),
        m_hist.axis(3).index(eta_deg),
        m_hist.axis(4).index(phi_deg)
      }
    ];
    if ((entry_table.prob_electron != entry_hist.prob_electron)
        || (entry_table.prob_pion != entry_hist.prob_pion)
        || (entry_table.prob_kaon != entry_hist.prob_kaon)
        || (entry_table.prob_proton != entry_hist.prob_proton)) {
      error("Consistency error between table and hist");
      error("electron: {} vs {}", entry_table.prob_electron, entry_hist.prob_electron);
      error("pion: {} vs {}", entry_table.prob_pion, entry_hist.prob_pion);
      error("kaon: {} vs {}", entry_table.prob_kaon, entry_hist.prob_kaon);
      error("proton: {} vs {}", entry_table.prob_proton, entry_hist.prob_proton);
      std::terminate();
    }

    return &m_table.at(index);

}

void PIDLookupTable::LoadFile(const std::string& filename) {

    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Unable to open LUT file!");
    }

    std::string line;
    std::istringstream iss;
    double step;

    do { std::getline(file, line); } while (line.empty() || line[0] == '#');
    debug("Parsing pdg binning: {}", line);
    iss.str(line);
    iss.clear();
    std::copy(std::istream_iterator<int>(iss), std::istream_iterator<int>(), std::back_inserter(m_pdg_binning));
    bh::axis::category<int> pdg_bins(m_pdg_binning);

    do { std::getline(file, line); } while (line.empty() || line[0] == '#');
    debug("Parsing charge binning: {}", line);
    iss.str(line);
    iss.clear();
    std::copy(std::istream_iterator<int>(iss), std::istream_iterator<int>(), std::back_inserter(m_charge_binning));
    bh::axis::category<int> charge_bins(m_charge_binning);

    do { std::getline(file, line); } while (line.empty() || line[0] == '#');
    debug("Parsing momentum binning: {}", line);
    iss.str(line);
    iss.clear();
    double lower_bound, upper_bound;
    if (!(iss >> lower_bound
              >> upper_bound
              >> step)) {
        error("Unable to parse line: {}", line);
        throw std::runtime_error("Unable to parse momentum binning");
    }
    m_momentum_binning.lower_bound = lower_bound;
    m_momentum_binning.upper_bound = upper_bound;
    m_momentum_binning.bin_count = static_cast<int>((upper_bound - lower_bound) / step);
    bh::axis::regular<> momentum_bins(bh::axis::step(step), lower_bound, upper_bound);

    do { std::getline(file, line); } while (line.empty() || line[0] == '#');
    debug("Parsing eta binning: {}", line);
    iss.str(line);
    iss.clear();
    if (!(iss >> lower_bound
              >> upper_bound
              >> step)) {
        error("Unable to parse line: {}", line);
        throw std::runtime_error("Unable to parse eta binning");
    }
    m_eta_binning.lower_bound = lower_bound;
    m_eta_binning.upper_bound = upper_bound;
    m_eta_binning.bin_count = static_cast<int>((upper_bound - lower_bound) / step);
    bh::axis::regular<> polar_bins(bh::axis::step(step), lower_bound, upper_bound);

    do { std::getline(file, line); } while (line.empty() || line[0] == '#');
    debug("Parsing phi binning: ", line);
    iss.str(line);
    iss.clear();
    if (!(iss >> lower_bound
              >> upper_bound
              >> step)) {
        error("Unable to parse line: ", line);
        throw std::runtime_error("Unable to parse phi binning");
    }
    m_phi_binning.lower_bound = lower_bound;
    m_phi_binning.upper_bound = upper_bound;
    m_phi_binning.bin_count = static_cast<int>((upper_bound - lower_bound) / step);
    bh::axis::circular<> azimuthal_bins(bh::axis::step(step), lower_bound, upper_bound);

    m_hist = bh::make_histogram_with(bh::dense_storage<PIDLookupTable::Entry>(), pdg_bins, charge_bins, momentum_bins, polar_bins, azimuthal_bins);

    while (std::getline(file, line)) {
        Entry entry;
        if (line.empty() || line[0] == '#') continue;

        iss.str(line);
        iss.clear();
        // Read each field from the line and assign to Entry struct members
        if (iss >> entry.pdg
                >> entry.charge
                >> entry.momentum
                >> entry.eta
                >> entry.phi
                >> entry.prob_electron
                >> entry.prob_pion
                >> entry.prob_kaon
                >> entry.prob_proton) {

            m_table.push_back(std::move(entry));

	    // operator() here allows to lookup mutable entry and increases the access counter
	    auto &entry_hist = *m_hist(
              entry.pdg,
              entry.charge,
              entry.momentum + momentum_bins.bin(0).width() / 2,
              entry.eta + polar_bins.bin(0).width() / 2,
              entry.phi + azimuthal_bins.bin(0).width() / 2
            );
            entry_hist.prob_electron = entry.prob_electron;
            entry_hist.prob_pion = entry.prob_pion;
            entry_hist.prob_kaon = entry.prob_kaon;
            entry_hist.prob_proton = entry.prob_proton;
        }
        else {
            error("Unable to parse LUT file!");
            throw std::runtime_error("Unable to parse LUT file!");
        }
    }
    size_t expected_table_size = m_momentum_binning.bin_count * m_eta_binning.bin_count *
                                 m_phi_binning.bin_count * m_charge_binning.size() * m_pdg_binning.size();
    if (expected_table_size != m_table.size()) {
        error("Wrong number of entries in table for given bin counts. Expected {} got {}", expected_table_size, m_table.size());
        throw std::runtime_error("Wrong number of entries in table for given bin counts");
    }
    for (auto&& b : bh::indexed(m_hist)) {
      if (b->value() != 1) {
        error(
          "Bin {} {} {}:{} {}:{} {}:{} is defined {} times in the PID table",
          b.bin(0).lower(),
          b.bin(1).lower(),
          b.bin(2).lower(),
          b.bin(2).upper(),
          b.bin(3).lower(),
          b.bin(3).upper(),
          b.bin(4).lower(),
          b.bin(4).upper(),
          b->value()
        );
      }
    }
    file.close();
}

void PIDLookupTable::AppendEntry(Entry&& entry) {
    m_table.push_back(std::move(entry));
}


std::optional<size_t> PIDLookupTable::FindBin(const Binning& binning, double value) {

    if ((value < binning.lower_bound) || value >= binning.upper_bound) {
        return std::nullopt;
    }
    double scaled_unit_interval = (value - binning.lower_bound) / (binning.upper_bound - binning.lower_bound);
    return scaled_unit_interval * binning.bin_count;
}


std::optional<size_t> PIDLookupTable::FindBin(const std::vector<int>& binning, int value) {
    for (int i=0; i<binning.size(); ++i) {
        if (binning[i] == value) return i;
    }
    return std::nullopt;
}

}
