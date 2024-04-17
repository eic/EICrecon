// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Nathan Brei

#pragma once

#include <optional>
#include <vector>
#include <string>
#include <stddef.h>

class PIDLookupTable {

public:
    struct Entry {
        int pdg, charge;
        double momentum, eta, phi;
        double prob_electron, prob_pion, prob_kaon, prob_proton;
    };

    struct Binning {
        double lower_bound;
        double upper_bound;
        size_t bin_count;
    };


private:
    std::vector<Entry> m_table;

    std::vector<int> m_pdg_binning;
    std::vector<int> m_charge_binning;
    Binning m_momentum_binning;
    Binning m_eta_binning;
    Binning m_phi_binning;

public:

    const Entry* Lookup(int pdg, int charge, double momentum, double eta_deg, double phi_deg) const;

    std::vector<int>& GetPDGBinning() { return m_pdg_binning; }
    std::vector<int>& GetChargeBinning() { return m_charge_binning; }
    Binning& GetMomentumBinning() { return m_momentum_binning; }
    Binning& GetEtaBinning() { return m_eta_binning; }
    Binning& GetPhiBinning() { return m_phi_binning; }

    const std::vector<int>& GetPDGBinning() const { return m_pdg_binning; }
    const std::vector<int>& GetChargeBinning() const { return m_charge_binning; }
    const Binning& GetMomentumBinning() const { return m_momentum_binning; }
    const Binning& GetEtaBinning() const { return m_eta_binning; }
    const Binning& GetPhiBinning() const { return m_phi_binning; }

    void LoadFile(const std::string& filename);
    void AppendEntry(Entry&& entry);

    static std::optional<size_t> FindBin(const Binning& binning, double value);
    static std::optional<size_t> FindBin(const std::vector<int>& binning, int value);

};
