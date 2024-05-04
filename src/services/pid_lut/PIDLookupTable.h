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

private:
    boost::histogram::histogram<
      std::tuple<
        boost::histogram::axis::category<int>,
        boost::histogram::axis::category<int>,
        boost::histogram::axis::regular<>,
        boost::histogram::axis::regular<>,
        boost::histogram::axis::circular<>
      >
      , boost::histogram::dense_storage<Entry>
    > m_hist;

public:

    PIDLookupTable() : algorithms::LoggerMixin("PIDLookupTable") {};

    const Entry* Lookup(int pdg, int charge, double momentum, double eta_deg, double phi_deg) const;

    void LoadFile(const std::string& filename);
};

}
