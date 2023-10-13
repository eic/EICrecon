// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng, Jihee Kim, Sylvester Joosten, Whitney Armstrong, Wouter Deconinck, David Lawrence

/*
 *  An algorithm to group readout hits from a calorimeter
 *  Energy is summed
 *
 *  Author: Chao Peng (ANL), 03/31/2021
 */

#include "algorithms/calorimetry/CalorimeterHitsMerger.h"

namespace eicrecon {

void CalorimeterHitsMerger::init(const dd4hep::Detector* detector, const dd4hep::rec::CellIDPositionConverter* converter, std::shared_ptr<spdlog::logger>& logger) {
    m_detector = detector;
    m_converter = converter;
    m_log = logger;

    if (m_cfg.readout.empty()) {
        m_log->error("readoutClass is not provided, it is needed to know the fields in readout ids");
        return;
    }

    try {
        auto id_desc = m_detector->readout(m_cfg.readout).idSpec();
        id_mask = 0;
        std::vector<std::pair<std::string, int>> ref_fields;
        for (size_t i = 0; i < m_cfg.fields.size(); ++i) {
            id_mask |= id_desc.field(m_cfg.fields[i])->mask();
            // use the provided id number to find ref cell, or use 0
            int ref = i < m_cfg.refs.size() ? m_cfg.refs[i] : 0;
            ref_fields.emplace_back(m_cfg.fields[i], ref);
        }
        ref_mask = id_desc.encode(ref_fields);
    } catch (...) {
        auto mess = fmt::format("Failed to load ID decoder for {}", m_cfg.readout);
        m_log->warn(mess);
//        throw std::runtime_error(mess);
    }
    id_mask = ~id_mask;
    m_log->debug("ID mask in {:s}: {:#064b}", m_cfg.readout, id_mask);
}

std::unique_ptr<edm4eic::CalorimeterHitCollection> CalorimeterHitsMerger::process(const edm4eic::CalorimeterHitCollection &input) {
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
    auto volman = m_detector->volumeManager();

    for (const auto &[id, ixs] : merge_map) {
        // reference fields id
        const uint64_t ref_id = id | ref_mask;
        // global positions
        const auto gpos = m_converter->position(ref_id);
        // local positions
        auto alignment = volman.lookupDetElement(ref_id).nominal();
        const auto pos = alignment.worldToLocal(dd4hep::Position(gpos.x(), gpos.y(), gpos.z()));
        m_log->debug("{}, {}", volman.lookupDetElement(ref_id).path(), volman.lookupDetector(ref_id).path());
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

    m_log->debug("Size before = {}, after = {}", input.size(), output->size());

    return output;
}

} // namespace eicrecon
