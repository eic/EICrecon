// Copyright 2024, Nathan Brei
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once
#include <optional>

class PIDLookupTable {

public:
    struct Entry {
        double prob_electron, prob_pion, prob_kaon, prob_proton;
        int pdg, charge;
        double momentum, theta, phi;
    }

    struct Binning {
        double lower_bound;
        double upper_bound; 
        double step;
    }


private:
    std::vector<Entry> m_table;

    Binning m_momentum_binning;
    Binning m_theta_binning;
    Binning m_phi_binning;

public:

    std::optional<const Entry&> Lookup(int pdg, int charge, TVector3 momentum);
    std::optional<const Entry&> Lookup(int pdg, int charge, double momentum, double polar_theta_deg, double azimuthal_phi_deg);

    Binning& GetMomentumBinning() { return m_momentum_binning; }
    Binning& GetThetaBinning() { return m_theta_binning; }
    Binning& GetPhiBinning() { return m_phi_binning; }

    void LoadFile(std::string& filename);
    void AppendEntry(Entry&& entry);

    std::optional<size_t> FindBin(const Binning& binning, double value);

}


