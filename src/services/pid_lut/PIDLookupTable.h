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
        double momentum, theta, phi;
        double prob_electron, prob_pion, prob_kaon, prob_proton;
    };

    struct Binning {
        double lower_bound;
        double upper_bound; 
        double bin_count;
    };


private:
    std::vector<Entry> m_table;

    Binning m_momentum_binning;
    Binning m_theta_binning;
    Binning m_phi_binning;

public:

    const Entry* Lookup(int pdg, int charge, TVector3 momentum);
    const Entry* Lookup(int pdg, int charge, double momentum, double polar_theta_deg, double azimuthal_phi_deg);

    Binning& GetMomentumBinning() { return m_momentum_binning; }
    Binning& GetThetaBinning() { return m_theta_binning; }
    Binning& GetPhiBinning() { return m_phi_binning; }

    void LoadFile(std::string& filename);
    void AppendEntry(Entry&& entry);

    static std::optional<size_t> FindBin(const Binning& binning, double value);

};


