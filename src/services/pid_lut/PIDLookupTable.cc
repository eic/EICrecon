// Copyright 2024, Nathan Brei
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "PIDLookupTable.h"


const PIDLookupTable::Entry* PIDLookupTable::Lookup(int pdg, int charge, TVector3 momentum) {
    return nullptr;
}

const PIDLookupTable::Entry* PIDLookupTable::Lookup(int pdg, int charge, double momentum, double polar_theta_deg, double azimuthal_phi_deg) {
    size_t pdg_bin;
    switch(pdg) {
        case 1: pdg_bin = 0; break;
        default: return nullptr;
    }
    size_t charge_bin;
    size_t charge_bin_count = 2;
    switch(charge) {
        case -1: charge_bin = 0; break;
        case 1:  charge_bin = 1; break;
        default: return nullptr;
    }
    auto momentum_bin = FindBin(m_momentum_binning, momentum);
    if (!momentum_bin.has_value()) return nullptr;

    auto theta_bin = FindBin(m_theta_binning, polar_theta_deg);
    if (!theta_bin.has_value()) return nullptr;

    auto phi_bin = FindBin(m_phi_binning, azimuthal_phi_deg);
    if (!phi_bin.has_value()) return nullptr;

    size_t offset = 1;
    size_t index = *phi_bin * offset;

    offset *= m_phi_binning.bin_count;
    index += *theta_bin * offset;

    offset *= m_theta_binning.bin_count;
    index += *momentum_bin * offset;

    offset *= m_momentum_binning.bin_count;
    index += charge_bin * offset;

    offset *= charge_bin_count;
    index += pdg_bin * offset;

    return &m_table.at(index);

}

void PIDLookupTable::LoadFile(std::string& filename) {

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


