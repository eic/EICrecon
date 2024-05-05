// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Nathan Brei, Dmitry Kalinkin

#pragma once

#include <algorithms/logger.h>
#include <boost/histogram.hpp>
#include <optional>
#include <vector>
#include <string>
#include <stddef.h>

namespace eicrecon {

class PIDLookupTable : public algorithms::LoggerMixin {

public:
    /// The histogram entry with access counter, where the probabilities are stored in metadata
    struct Entry : public boost::histogram::accumulators::count<unsigned char, false> {
        double prob_electron, prob_pion, prob_kaon, prob_proton;
    };

    struct Binning {
      std::vector<int> pdg_values;
      std::vector<int> charge_values;
      std::vector<double> momentum_edges;
      std::vector<double> polar_edges;
      std::vector<double> azimuthal_binning;
      bool polar_bin_centers_in_lut;
      bool skip_legacy_header;
      bool use_radians;
    };

private:
    boost::histogram::histogram<
      std::tuple<
        boost::histogram::axis::category<int>,
        boost::histogram::axis::category<int>,
        boost::histogram::axis::variable<>,
        boost::histogram::axis::variable<>,
        boost::histogram::axis::circular<>
      >
      , boost::histogram::dense_storage<Entry>
    > m_hist;
    bool m_symmetrizing_charges;

public:

    PIDLookupTable() : algorithms::LoggerMixin("PIDLookupTable") {};

    const Entry* Lookup(int pdg, int charge, double momentum, double eta_deg, double phi_deg) const;

    void load_file(const std::string& filename, const Binning &binning);
};

}
