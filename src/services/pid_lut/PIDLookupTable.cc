// Copyright 2024, Nathan Brei
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "PIDLookupTable.h"
#include <iostream>


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

void PIDLookupTable::LoadFile(const std::string& filename) {

    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Unable to open LUT file!");
    }

    std::string line;
    std::istringstream iss;
    double step;

    // Read header
    while (line.empty() || line[0] == '#') std::getline(file, line);
    std::cout << "Parsing line 1: " << line << std::endl;
    iss.str(line);
    iss.clear();
    if (!(iss >> m_momentum_binning.lower_bound
              >> m_momentum_binning.upper_bound
              >> step)) {
        std::cout << "Unable to parse line: " << line << std::endl;
        throw std::runtime_error("Unable to parse header line 1");
    }
    m_momentum_binning.bin_count = (m_momentum_binning.upper_bound - m_momentum_binning.lower_bound) / step;

    do { std::getline(file, line); } while (line.empty() || line[0] == '#');
    std::cout << "Parsing line 2: " << line << std::endl;
    iss.str(line);
    iss.clear();
    if (!(iss >> m_theta_binning.lower_bound
              >> m_theta_binning.upper_bound
              >> step)) {
        std::cout << "Unable to parse line: " << line << std::endl;
        throw std::runtime_error("Unable to parse header line 2");
    }
    m_theta_binning.bin_count = (m_theta_binning.upper_bound - m_theta_binning.lower_bound) / step;

    line = "";
    while (line.empty() || line[0] == '#') std::getline(file, line);
    std::cout << "Parsing line 3: " << line << std::endl;
    iss.str(line);
    iss.clear();
    if (!(iss >> m_phi_binning.lower_bound
              >> m_phi_binning.upper_bound
              >> step)) {
        std::cout << "Unable to parse line: " << line << std::endl;
        throw std::runtime_error("Unable to parse header line 3");
    }
    m_phi_binning.bin_count = (m_phi_binning.upper_bound - m_phi_binning.lower_bound) / step;


    while (std::getline(file, line)) {
        Entry entry;
        if (line.empty() || line[0] == '#') continue;

        iss.str(line);
        iss.clear();
        // Read each field from the line and assign to Entry struct members
        if (iss >> entry.pdg
                >> entry.charge
                >> entry.momentum
                >> entry.theta
                >> entry.phi
                >> entry.prob_electron
                >> entry.prob_pion
                >> entry.prob_kaon
                >> entry.prob_proton) {

            m_table.push_back(std::move(entry));
        }
        else {
            throw std::runtime_error("Unable to parse LUT file!");
        }
    }
    size_t expected_table_size = m_momentum_binning.bin_count * m_theta_binning.bin_count * 
                                 m_phi_binning.bin_count * 4 * 2; 
    if (expected_table_size != m_table.size()) {
        std::cout << "Wrong number of entries in table for given bin counts. Expected " << expected_table_size << ", got " << m_table.size() << std::endl;
        throw std::runtime_error("Wrong number of entries in table for given bin counts");
    }
    file.close();
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


