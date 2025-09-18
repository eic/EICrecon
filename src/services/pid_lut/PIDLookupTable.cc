// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Nathan Brei, Dmitry Kalinkin

#include "services/pid_lut/PIDLookupTable.h"

#include <boost/histogram.hpp>
#include <boost/iostreams/close.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <fmt/core.h>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
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
  //Print polar edges after fudge
  // info("Polar edges after fudge: {}", fmt::join(polar_edges, ", "));
  bh::axis::variable<double, bh::use_default, bh::axis::option::none_t> polar_bins(polar_edges);
  //Print the polar_bins themselves
  //   std::vector<std::string> polar_edges_str;
  // for (size_t i = 0; i <= polar_bins.size(); ++i) {
  //     const auto& bin = polar_bins.bin(i);
  //     polar_edges_str.push_back(fmt::format("[{}, {})", bin.lower(), bin.upper()));
  // }
  // info("Polar bins: {}", fmt::join(polar_edges_str, ", "));
  bh::axis::circular<> azimuthal_bins(bh::axis::step(binning.azimuthal_binning.at(2) * angle_fudge),
                                      binning.azimuthal_binning.at(0) * angle_fudge,
                                      binning.azimuthal_binning.at(1) * angle_fudge);

  m_hist = bh::make_histogram_with(bh::dense_storage<PIDLookupTable::Entry>(), pdg_bins,
                                   charge_bins, momentum_bins, polar_bins, azimuthal_bins);

  // Print the binning for verification
  // info("PID LUT binning:");
  // info("  PDG values: {}", fmt::join(binning.pdg_values, ", "));
  // info("  Charge values: {}", fmt::join(binning.charge_values, ", "));
  // info("  Momentum edges: {}", fmt::join(binning.momentum_edges, ", "));
  // info("  Polar edges: {}", fmt::join(binning.polar_edges, ", "));
  // info("  Azimuthal binning: lower={}, upper={}, step={}", binning.azimuthal_binning.at(0),
  //       binning.azimuthal_binning.at(1), binning.azimuthal_binning.at(2));

  // Print the bin ranges from the bh::axis objects
  // info("  Momentum bins: {}", fmt::join(momentum_bins.edges(), ", "));
  // {
  //   std::vector<std::string> polar_intervals;
  //   for (size_t i = 0; i < polar_bins.size(); ++i) {
  //     const auto& bin = polar_bins.bin(i);
  //     polar_intervals.push_back(fmt::format("[{}, {})", bin.lower(), bin.upper()));
  //   }
  //   info("  Polar bins: {}", fmt::join(polar_intervals, ", "));
  // }
  // {
  //   std::vector<std::string> azimuthal_intervals;
  //   for (size_t i = 0; i < azimuthal_bins.size(); ++i) {
  //     const auto& bin = azimuthal_bins.bin(i);
  //     azimuthal_intervals.push_back(fmt::format("[{}, {})", bin.lower(), bin.upper()));
  //   }
  //   info("  Azimuthal bins: {}", fmt::join(azimuthal_intervals, ", "));
  // }

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

      //
      // error("Filling LUT with pdg={}, charge={}, momentum={:.2f}, theta={:.2f}, phi={:.2f},     "
      //       "prob_electron={}, prob_pion={}, prob_kaon={}, prob_proton={}",
      //       pdg, charge, momentum, eta, phi, prob_electron, prob_pion, prob_kaon, prob_proton);

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
      // Print the parsed values for verification if the eta is larger than the maximum polar bins from m_hist
      // if (eta * angle_fudge < polar_bins.bin(0).lower() ||
      //     eta * angle_fudge > polar_bins.bin(polar_bins.size() ).upper()) {
      //   error("Skipping LUT entry with out-of-bounds polar angle: pdg={}, charge={}, momentum={:.2f}, "
      //        "theta={:.2f}, phi={:.2f}, prob_electron={}, prob_pion={}, prob_kaon={}, prob_proton={}",
      //        pdg, charge, momentum, eta * angle_fudge, phi * angle_fudge, prob_electron, prob_pion,
      //        prob_kaon, prob_proton);
      //   error("Max polar bin edges are: {}, {}", polar_bins.bin(polar_bins.size()).lower(), polar_bins.bin(polar_bins.size()).upper());
      //   //Try and look at the properties of entry
      //   error("Entry bin edges are: pdg=[{}, {}), charge=[{}, {}), momentum=[{}, {}), polar=[{}, {}), azimuthal=[{}, {})",
      //         m_hist.axis(0).bin(m_hist.axis(0).index(pdg)).lower(), m_hist.axis(0).bin(m_hist.axis(0).index(pdg)).upper(),
      //         m_hist.axis(1).bin(m_hist.axis(1).index(charge)).lower(), m_hist.axis(1).bin(m_hist.axis(1).index(charge)).upper(),
      //         m_hist.axis(2).bin(m_hist.axis(2).index(momentum)).lower(), m_hist.axis(2).bin(m_hist.axis(2).index(momentum)).upper(),
      //         m_hist.axis(3).bin(m_hist.axis(3).index(eta * angle_fudge)).lower(), m_hist.axis(3).bin(m_hist.axis(3).index(eta * angle_fudge)).upper(),
      //         m_hist.axis(4).bin(m_hist.axis(4).index(phi * angle_fudge)).lower(), m_hist.axis(4).bin(m_hist.axis(4).index(phi * angle_fudge)).upper());
      // }

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
