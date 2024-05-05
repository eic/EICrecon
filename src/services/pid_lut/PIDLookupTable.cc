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

    return &m_hist[
      decltype(m_hist)::multi_index_type {
        m_hist.axis(0).index(pdg),
        m_hist.axis(1).index(charge),
        m_hist.axis(2).index(momentum),
        m_hist.axis(3).index(eta_deg),
        m_hist.axis(4).index(phi_deg)
      }
    ];
}

void PIDLookupTable::load_file(const std::string& filename, const PIDLookupTable::Binning &binning) {
    level(algorithms::LogLevel::kTrace);
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Unable to open LUT file!");
    }

    std::string line;
    std::istringstream iss;
    double step;

    do { std::getline(file, line); } while (line.empty() || line[0] == '#');
    debug("Parsing pdg binning: {}", line);

    do { std::getline(file, line); } while (line.empty() || line[0] == '#');
    debug("Ignoring charge binning: {}", line);

    do { std::getline(file, line); } while (line.empty() || line[0] == '#');
    debug("Ignoring momentum binning: {}", line);

    do { std::getline(file, line); } while (line.empty() || line[0] == '#');
    debug("Ignoring eta binning: {}", line);

    do { std::getline(file, line); } while (line.empty() || line[0] == '#');
    debug("Ignoring phi binning: ", line);

    bh::axis::category<int> pdg_bins(binning.pdg_values);
    bh::axis::category<int> charge_bins(binning.charge_values);
    bh::axis::regular<> momentum_bins(bh::axis::step(binning.momentum_binning.at(2)), binning.momentum_binning.at(0), binning.momentum_binning.at(1));
    bh::axis::regular<> polar_bins(bh::axis::step(binning.polar_binning.at(2)), binning.polar_binning.at(0), binning.polar_binning.at(1));
    bh::axis::circular<> azimuthal_bins(bh::axis::step(binning.azimuthal_binning.at(2)), binning.azimuthal_binning.at(0), binning.azimuthal_binning.at(1));

    m_hist = bh::make_histogram_with(bh::dense_storage<PIDLookupTable::Entry>(), pdg_bins, charge_bins, momentum_bins, polar_bins, azimuthal_bins);

    while (std::getline(file, line)) {
        Entry entry;
        if (line.empty() || line[0] == '#') continue;

        iss.str(line);
        iss.clear();
        double pdg, charge, momentum, eta, phi, prob_electron, prob_pion, prob_kaon, prob_proton;
        // Read each field from the line and assign to Entry struct members
        if (iss >> pdg
                >> charge
                >> momentum
                >> eta
                >> phi
                >> prob_electron
                >> prob_pion
                >> prob_kaon
                >> prob_proton) {

	    // operator() here allows to lookup mutable entry and increases the access counter
	    auto &entry = *m_hist(
              pdg,
              charge,
              momentum + momentum_bins.bin(0).width() / 2,
              eta + polar_bins.bin(0).width() / 2,
              phi + azimuthal_bins.bin(0).width() / 2
            );
            entry.prob_electron = prob_electron;
            entry.prob_pion = prob_pion;
            entry.prob_kaon = prob_kaon;
            entry.prob_proton = prob_proton;
        }
        else {
            error("Unable to parse LUT file!");
            throw std::runtime_error("Unable to parse LUT file!");
        }
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

}
