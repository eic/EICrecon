// Copyright 2024, Nathan Brei
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "PIDLookupTable.h"


std::optional<const Entry&> PIDLookupTable::Lookup(int pdg, int charge, TVector3 momentum) {
    return std::nullopt;
}

std::optional<const Entry&> PIDLookupTable::Lookup(int pdg, int charge, double momentum, double polar_angle_theta_deg, double azimuthal_angle_phi_deg) {
    return std::nullopt;
}

void PIDLookupTable::LoadFile(std::string& filename) {

}

std::optional<size_t> PIDLookupTable::FindBin(const Binning& binning, double value) {

}


