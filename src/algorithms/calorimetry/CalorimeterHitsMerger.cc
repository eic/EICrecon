// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng, Jihee Kim, Sylvester Joosten, Whitney Armstrong, Wouter Deconinck, David Lawrence

/*
 *  An algorithm to group readout hits from a calorimeter
 *  Energy is summed
 *
 *  Author: Chao Peng (ANL), 03/31/2021
 */

#include <algorithms/calorimetry/CalorimeterHitsMerger.h>

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
        // debug() << fmt::format("Referece id mask for the fields {:#064b}", ref_mask) << endmsg;
    } catch (...) {
        auto mess = fmt::format("Failed to load ID decoder for {}", m_readout);
        m_log->error(mess);
        throw std::runtime_error(mess);
    }
    id_mask = ~id_mask;
    m_log->info(fmt::format("ID mask in {:s}: {:#064b}", m_readout, id_mask));
}

void CalorimeterHitsMerger::execute() {

    // find the hits that belong to the same group (for merging)
    std::unordered_map<long long, std::vector<const edm4eic::CalorimeterHit *>> merge_map;
    for (const auto &h: m_inputs) {
        int64_t id = h->getCellID() & id_mask;
        // use the reference field position
        auto it = merge_map.find(id);
        if (it == merge_map.end()) {
            merge_map[id] = {h};
        } else {
            it->second.push_back(h);
        }
    }

    // sort hits by energy from large to small
    std::for_each(merge_map.begin(), merge_map.end(), [](auto &it) {
        std::sort(it.second.begin(), it.second.end(), [](const auto &h1, const auto &h2) {
            return h1->getEnergy() > h2->getEnergy();
        });
    });

    // reconstruct info for merged hits
    // dd4hep decoders
    auto poscon = m_geoSvc->cellIDPositionConverter();
    auto volman = m_geoSvc->detector()->volumeManager();

    for (auto &[id, hits]: merge_map) {
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
        for (auto &hit: hits) {
            energy += hit->getEnergy();
            energyError += hit->getEnergyError() * hit->getEnergyError();
            time += hit->getTime();
            timeError += hit->getTimeError() * hit->getTimeError();
        }
        energyError = sqrt(energyError);
        time /= hits.size();
        timeError = sqrt(timeError) / hits.size();

        const auto &href = hits.front();

        // create const vectors for passing to hit initializer list
        const decltype(edm4eic::CalorimeterHitData::position) position(
                gpos.x() / dd4hep::mm, gpos.y() / dd4hep::mm, gpos.z() / dd4hep::mm
        );
        const decltype(edm4eic::CalorimeterHitData::local) local(
                pos.x(), pos.y(), pos.z()
        );

        m_outputs.push_back(
                new edm4eic::CalorimeterHit{
                        href->getCellID(),
                        energy,
                        energyError,
                        time,
                        timeError,
                        position,
                        href->getDimension(),
                        href->getSector(),
                        href->getLayer(),
                        local}); // Can do better here? Right now position is mapped on the central hit
    }

    m_log->debug(fmt::format("Size before = {}, after = {}", m_inputs.size(), m_outputs.size()) );
}


