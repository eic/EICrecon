// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng, Jihee Kim, Sylvester Joosten, Whitney Armstrong, Wouter Deconinck, David Lawrence, Derek Anderson

/*
 *  An algorithm to group readout hits from a calorimeter
 *  Energy is summed
 *
 *  Author: Chao Peng (ANL), 03/31/2021
 */

#include "algorithms/calorimetry/CalorimeterHitsMerger.h"

#include <DD4hep/Alignments.h>
#include <DD4hep/DetElement.h>
#include <DD4hep/Objects.h>
#include <DD4hep/Readout.h>
#include <DD4hep/VolumeManager.h>
#include <DDSegmentation/BitFieldCoder.h>
#include <edm4eic/CalorimeterHit.h>
#include <Evaluator/DD4hepUnits.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <fmt/core.h>
#include <algorithm>
#include <algorithms/service.h>
#include <cmath>
#include <cstddef>
#include <gsl/pointers>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "algorithms/calorimetry/CalorimeterHitsMergerConfig.h"
#include "services/evaluator/EvaluatorSvc.h"

namespace eicrecon {

void CalorimeterHitsMerger::init() {

    if (m_cfg.readout.empty()) {
        error("readoutClass is not provided, it is needed to know the fields in readout ids");
        return;
    }

    // check that input field and transformation vectors
    // are the same length
    if (m_cfg.fields.size() != m_cfg.fieldTransformations.size()) {
      error(
        "Size of field and transformation vectors are different ({} vs. {}).",
        m_cfg.fields.size(),
        m_cfg.fieldTransformations.size()
      );
      return;
    }

    // initialize descriptor + decoders
    try {
      id_desc = m_detector->readout(m_cfg.readout).idSpec();
      id_decoder = id_desc.decoder();
      for (const auto& field : m_cfg.fields) {
        const short index = id_decoder->index(field);
      }
    } catch (...) {
      auto mess = fmt::format("Failed to load ID decoder for {}", m_cfg.readout);
      warning(mess);
//        throw std::runtime_error(mess);
    }

    // lambda to translate IDDescriptor fields into function parameters
    std::function hit_transform = [this](const edm4eic::CalorimeterHit& hit) {
      std::unordered_map<std::string, double> params;
      for (const auto& name_field : id_desc.fields()) {
        params.emplace(name_field.first, name_field.second->value(hit.getCellID()));
        trace("{} = {}", name_field.first, name_field.second->value(hit.getCellID()));
      }
      return params;
    };

    // loop through provided readout fields
    auto& svc = algorithms::ServiceSvc::instance();
    for (std::size_t iField = 0; std::string& field : m_cfg.fields) {

      // grab provided transformation and field
      std::string field_transform = m_cfg.fieldTransformations.at(iField);
      auto name_field = id_desc.field(field);

      // set transformation for each field
      ref_maps[field] = svc.service<EvaluatorSvc>("EvaluatorSvc")->compile(field_transform, hit_transform);
      trace("{}: using transformation '{}'", field, field_transform);
      ++iField;
    }  // end field loop

}

void CalorimeterHitsMerger::process(
      const CalorimeterHitsMerger::Input& input,
      const CalorimeterHitsMerger::Output& output) const {

    const auto [in_hits] = input;
    auto [out_hits] = output;

    // find the hits that belong to the same group (for merging)
    MergeMap merge_map;
    build_merge_map(in_hits, merge_map);
    debug("Merge map built: merging {} hits into {} merged hits", in_hits->size(), merge_map.size());

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

void CalorimeterHitsMerger::build_merge_map(
      const edm4eic::CalorimeterHitCollection* in_hits,
      MergeMap& merge_map) const {

  std::vector<RefField> ref_fields;
  for (std::size_t iHit = 0; const auto& hit : *in_hits) {

    ref_fields.clear();
    for (std::size_t iField = 0; const auto& name_field : id_desc.fields()) {

      // apply mapping to field if provided,
      // otherwise copy value of field
      if (ref_maps.count(name_field.first) > 0) {
        ref_fields.push_back(
          {name_field.first, ref_maps[name_field.first](hit)}
        );
      } else {
        ref_fields.push_back(
          {name_field.first, id_decoder->get(hit.getCellID(), name_field.first)}
        );
      }
      ++iField;
    }

    // encode new cell ID and add hit to map
    const uint64_t ref_id = id_desc.encode(ref_fields);
    merge_map[ref_id].push_back(iHit);
    ++iHit;

  }  // end hit loop

}  // end 'build_merge_map(edm4eic::CalorimeterHitsCollection*, MergeMap&)'

} // namespace eicrecon
