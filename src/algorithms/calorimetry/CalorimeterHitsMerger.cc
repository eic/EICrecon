// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng, Jihee Kim, Sylvester Joosten, Whitney Armstrong, Wouter Deconinck, David Lawrence

/*
 *  An algorithm to group readout hits from a calorimeter
 *  Energy is summed
 *
 *  Author: Chao Peng (ANL), 03/31/2021
 */

#include "algorithms/calorimetry/CalorimeterHitsMerger.h"

void CalorimeterHitsMerger::initialize() {

    if (m_readout.empty()) {
        m_log->error("readoutClass is not provided, it is needed to know the fields in readout ids");
        return;
    }

    try {
        auto id_desc = m_geoSvc->detector()->readout(m_readout).idSpec();
        id_mask = 0;
        std::vector<std::pair<std::string, int>> ref_fields;
        for (size_t i = 0; i < u_fields.size(); ++i) {
            id_mask |= id_desc.field(u_fields[i])->mask();
            // use the provided id number to find ref cell, or use 0
            int ref = i < u_refs.size() ? u_refs[i] : 0;
            ref_fields.emplace_back(u_fields[i], ref);
        }
        ref_mask = id_desc.encode(ref_fields);
        // debug() << fmt::format("Reference id mask for the fields {:#064b}", ref_mask) << endmsg;
    } catch (...) {
        auto mess = fmt::format("Failed to load ID decoder for {}", m_readout);
        m_log->warn(mess);
//        throw std::runtime_error(mess);
    }
    id_mask = ~id_mask;
    m_log->info(fmt::format("ID mask in {:s}: {:#064b}", m_readout, id_mask));
}

std::unique_ptr<edm4eic::CalorimeterHitCollection> CalorimeterHitsMerger::execute(const edm4eic::CalorimeterHitCollection &input) {
    auto output = std::make_unique<edm4eic::CalorimeterHitCollection>();

    // find the hits that belong to the same group (for merging)
    std::unordered_map<uint64_t, std::vector<std::size_t>> merge_map;
    std::size_t ix = 0;
    for (const auto &h : input) {
        uint64_t id = h.getCellID() & id_mask;
        merge_map[id].push_back(ix);

        ix++;
    }

    // sort hits by energy from large to small
    for (auto &it : merge_map) {
        std::sort(it.second.begin(), it.second.end(), [&](std::size_t ix1, std::size_t ix2) {
            return input[ix1].getEnergy() > input[ix2].getEnergy();
        });
    }

    // reconstruct info for merged hits
    // dd4hep decoders
    auto poscon = m_geoSvc->cellIDPositionConverter();
    auto volman = m_geoSvc->detector()->volumeManager();

    for (const auto &[id, ixs] : merge_map) {
        // reference fields id
        const uint64_t ref_id = id | ref_mask;
        // global positions
        const auto gpos = poscon->position(ref_id);
        // local positions
        auto alignment = volman.lookupDetElement(ref_id).nominal();
        const auto pos = alignment.worldToLocal(dd4hep::Position(gpos.x(), gpos.y(), gpos.z()));
        m_log->debug( fmt::format("{}, {}", volman.lookupDetElement(ref_id).path(), volman.lookupDetector(ref_id).path()) );
        // sum energy
        float energy = 0.;
        float energyError = 0.;
        float time = 0;
        float timeError = 0;
        for (auto ix : ixs) {
            auto hit = input[ix];
            energy += hit.getEnergy();
            energyError += hit.getEnergyError() * hit.getEnergyError();
            time += hit.getTime();
            timeError += hit.getTimeError() * hit.getTimeError();
        }
        energyError = sqrt(energyError);
        time /= ixs.size();
        timeError = sqrt(timeError) / ixs.size();

        const auto href = input[ixs.front()];

        // create const vectors for passing to hit initializer list
        const decltype(edm4eic::CalorimeterHitData::position) position(
                gpos.x() / dd4hep::mm, gpos.y() / dd4hep::mm, gpos.z() / dd4hep::mm
        );
        const decltype(edm4eic::CalorimeterHitData::local) local(
                pos.x(), pos.y(), pos.z()
        );

        output->create(
                        href.getCellID(),
                        energy,
                        energyError,
                        time,
                        timeError,
                        position,
                        href.getDimension(),
                        href.getSector(),
                        href.getLayer(),
                        local); // Can do better here? Right now position is mapped on the central hit
    }

    m_log->debug(fmt::format("Size before = {}, after = {}", input.size(), output->size()) );

    return output;
}
