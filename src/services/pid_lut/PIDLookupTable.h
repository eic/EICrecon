// Copyright 2024, Nathan Brei
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once
#include <TVector3.h>

#include <optional>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

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

    void LoadFile(const std::string& filename);
    void AppendEntry(Entry&& entry);

    static std::optional<size_t> FindBin(const Binning& binning, double value);
    static std::optional<size_t> FindBin(const std::vector<int>& binning, int value);

};


