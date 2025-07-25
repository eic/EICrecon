// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Nathan Brei, Dmitry Kalinkin

#include "services/pid_lut/PIDLookupTable.h"

#include <algorithm>
#include <boost/histogram.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/close.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <fmt/core.h>
#include <fstream> // IWYU pragma: keep
#include <iterator>
#include <sstream> // IWYU pragma: keep
#include <stdexcept>
// IWYU pragma: no_include <boost/mp11/detail/mp_defer.hpp>

namespace bh = boost::histogram;

namespace eicrecon {

const PIDLookupTable::Entry* PIDLookupTable::Lookup(int pdg, int charge, double momentum,
                                                    double theta_deg, double phi_deg) const {
  // Our lookup table expects _unsigned_ PDGs. The charge information is passed separately.
  pdg = std::abs(pdg);

  if (m_symmetrizing_charges) {
    charge = std::abs(charge);
  }

  return &m_hist[decltype(m_hist)::multi_index_type{
      m_hist.axis(0).index(pdg), m_hist.axis(1).index(charge), m_hist.axis(2).index(momentum),
      m_hist.axis(3).index(theta_deg), m_hist.axis(4).index(phi_deg)}];
}

void PIDLookupTable::load_file(const std::string& filename,
                               const PIDLookupTable::Binning& binning) {
  bool is_compressed = filename.substr(filename.length() - 3) == ".gz";

  std::ios_base::openmode mode = std::ios_base::in;
  if (is_compressed) {
    mode |= std::ios_base::binary;
  }

  std::ifstream file(filename, mode);
  if (!file) {
    throw std::runtime_error("Unable to open LUT file!");
  }
  boost::iostreams::filtering_istream in;
  if (is_compressed) {
    in.push(boost::iostreams::gzip_decompressor());
  }
  in.push(file);

  std::string line;
  std::istringstream iss;

  const double angle_fudge = binning.use_radians ? 180. / M_PI : 1.;

  bh::axis::category<int> pdg_bins(binning.pdg_values);
  bh::axis::category<int> charge_bins(binning.charge_values);
  bh::axis::variable<> momentum_bins(binning.momentum_edges);
  std::vector<double> polar_edges = binning.polar_edges;
  for (double& edge : polar_edges) {
    edge *= angle_fudge;
  }
  bh::axis::variable<> polar_bins(polar_edges);
  bh::axis::circular<> azimuthal_bins(bh::axis::step(binning.azimuthal_binning.at(2) * angle_fudge),
                                      binning.azimuthal_binning.at(0) * angle_fudge,
                                      binning.azimuthal_binning.at(1) * angle_fudge);

  m_hist = bh::make_histogram_with(bh::dense_storage<PIDLookupTable::Entry>(), pdg_bins,
                                   charge_bins, momentum_bins, polar_bins, azimuthal_bins);

  m_symmetrizing_charges = binning.charge_values.size() == 1;

  while (std::getline(in, line)) {
    Entry entry;
    if (line.empty() || line[0] == '#' ||
        std::all_of(std::begin(line), std::end(line),
                    [](unsigned char c) { return std::isspace(c); })) {
      continue;
    }

    iss.str(line);
    iss.clear();
    double pdg      = NAN;
    double charge   = NAN;
    double momentum = NAN;
    double eta;
    double phi;
    double prob_electron;
    double prob_pion;
    double prob_kaon;
    double prob_proton;
    // Read each field from the line and assign to Entry struct members
    if ((bool)(iss >> pdg >> charge >> momentum >> eta >> phi) &&
        (binning.missing_electron_prob || (bool)(iss >> prob_electron)) &&
        (bool)(iss >> prob_pion >> prob_kaon >> prob_proton)) {

      if (m_symmetrizing_charges) {
        charge = std::abs(charge);
      }

      // operator() here allows to lookup mutable entry and increases the access counter
      auto& entry = *m_hist(
          pdg, charge,
          momentum +
              (binning.momentum_bin_centers_in_lut ? 0. : (momentum_bins.bin(0).width() / 2)),
          eta * angle_fudge +
              (binning.polar_bin_centers_in_lut ? 0. : (polar_bins.bin(0).width() / 2)),
          phi * angle_fudge + (binning.azimuthal_bin_centers_in_lut
                                   ? 0.
                                   : (azimuthal_bins.bin(0).width() /
                                      2))); // N.B. bin(0) may not be of a correct width
      entry.prob_electron = prob_electron;
      entry.prob_pion     = prob_pion;
      entry.prob_kaon     = prob_kaon;
      entry.prob_proton   = prob_proton;
    } else {
      error("Unable to parse LUT file!");
      throw std::runtime_error("Unable to parse LUT file!");
    }
  }

  for (auto&& b : bh::indexed(m_hist)) {
    if (b->value() != 1) {
      error("Bin {} {} {}:{} {}:{} {}:{} is defined {} times in the PID table", b.bin(0).lower(),
            b.bin(1).lower(), b.bin(2).lower(), b.bin(2).upper(), b.bin(3).lower() / angle_fudge,
            b.bin(3).upper() / angle_fudge, b.bin(4).lower() / angle_fudge,
            b.bin(4).upper() / angle_fudge, b->value());
    }
  }

  boost::iostreams::close(in);
  file.close();
}

} // namespace eicrecon
