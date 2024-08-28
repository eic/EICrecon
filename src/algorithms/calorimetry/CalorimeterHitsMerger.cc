// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng, Jihee Kim, Sylvester Joosten, Whitney Armstrong, Wouter Deconinck, David Lawrence

/*
 *  An algorithm to group readout hits from a calorimeter
 *  Energy is summed
 *
 *  Author: Chao Peng (ANL), 03/31/2021
 */

#include "algorithms/calorimetry/CalorimeterHitsMerger.h"

#include <DD4hep/Alignments.h>
#include <DD4hep/DetElement.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Objects.h>
#include <DD4hep/Readout.h>
#include <DD4hep/VolumeManager.h>
#include <DDSegmentation/BitFieldCoder.h>
#include <Evaluator/DD4hepUnits.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <fmt/core.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <gsl/pointers>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "algorithms/calorimetry/CalorimeterHitsMergerConfig.h"

namespace eicrecon {

void CalorimeterHitsMerger::init() {

    if (m_cfg.readout.empty()) {
        error("readoutClass is not provided, it is needed to know the fields in readout ids");
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
        warning(mess);
//        throw std::runtime_error(mess);
    }
    id_mask = ~id_mask;
    debug("ID mask in {:s}: {:#064b}", m_cfg.readout, id_mask);
}

void CalorimeterHitsMerger::process(
      const CalorimeterHitsMerger::Input& input,
      const CalorimeterHitsMerger::Output& output) const {

    const auto [in_hits] = input;
    auto [out_hits] = output;

    // find the hits that belong to the same group (for merging)
    std::unordered_map<uint64_t, std::vector<std::size_t>> merge_map;
    std::size_t ix = 0;
    for (const auto &h : *in_hits) {
        uint64_t id = h.getCellID() & id_mask;
        merge_map[id].push_back(ix);

        ix++;
    }

    // sort hits by energy from large to small
    for (auto &it : merge_map) {
        std::sort(it.second.begin(), it.second.end(), [&](std::size_t ix1, std::size_t ix2) {
            return (*in_hits)[ix1].getEnergy() > (*in_hits)[ix2].getEnergy();
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
        debug("{}, {}", volman.lookupDetElement(ref_id).path(), volman.lookupDetector(ref_id).path());
        // sum energy
        float energy = 0.;
        float energyError = 0.;
        float time = 0;
        float timeError = 0;
        for (auto ix : ixs) {
            auto hit = (*in_hits)[ix];
            energy += hit.getEnergy();
            energyError += hit.getEnergyError() * hit.getEnergyError();
            time += hit.getTime();
            timeError += hit.getTimeError() * hit.getTimeError();
        }
        energyError = sqrt(energyError);
        time /= ixs.size();
        timeError = sqrt(timeError) / ixs.size();

        const auto href = (*in_hits)[ixs.front()];

        // create const vectors for passing to hit initializer list
        const decltype(edm4eic::CalorimeterHitData::position) position(
                gpos.x() / dd4hep::mm, gpos.y() / dd4hep::mm, gpos.z() / dd4hep::mm
        );
        const decltype(edm4eic::CalorimeterHitData::local) local(
                pos.x() / dd4hep::mm, pos.y() / dd4hep::mm, pos.z() / dd4hep::mm
        );

        out_hits->create(
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

    debug("Size before = {}, after = {}", in_hits->size(), out_hits->size());
}

} // namespace eicrecon
