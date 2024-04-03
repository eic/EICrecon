// Copyright 2024, Nathan Brei
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "PIDLookupTable.h"


const PIDLookupTable::Entry* PIDLookupTable::Lookup(int pdg, int charge, TVector3 momentum) {
    return nullptr;
}

const PIDLookupTable::Entry* PIDLookupTable::Lookup(int pdg, int charge, double momentum, double polar_angle_theta_deg, double azimuthal_angle_phi_deg) {
    return nullptr;
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
    int bin_count = (binning.upper_bound - binning.lower_bound) / binning.step;
    return scaled_unit_interval * bin_count;
}


