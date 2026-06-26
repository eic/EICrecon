// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck, Sylvester Joosten, Dmitry Romanov

#include <Evaluator/DD4hepUnits.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/Vector3d.h>
#include <edm4hep/Vector3f.h>
#include <podio/detail/Link.h>
#include <podio/detail/LinkCollectionImpl.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <gsl/pointers>
#include <memory>
#include <random>
#include <unordered_map>
#include <utility>

#include "SiliconTrackerDigi.h"
#include "algorithms/digi/SiliconTrackerDigiConfig.h"

namespace eicrecon {

void SiliconTrackerDigi::init() {}

void SiliconTrackerDigi::process(const SiliconTrackerDigi::Input& input,
                                 const SiliconTrackerDigi::Output& output) const {

  const auto [headers, sim_hits] = input;
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
  auto [raw_hits, links, associations] = output;
#else
  auto [raw_hits, associations] = output;
#endif

  // local random generator
  auto seed = m_uid.getUniqueID(*headers, name());
  std::default_random_engine generator(seed);
  std::normal_distribution<double> gaussian;

  // A map of unique cellIDs with temporary structure RawHit
  std::unordered_map<std::uint64_t, edm4eic::MutableRawTrackerHit> cell_hit_map;

  for (const auto& sim_hit : *sim_hits) {

    // time smearing
    double time_smearing = gaussian(generator) * m_cfg.timeResolution;
    double result_time   = sim_hit.getTime() + time_smearing;
    auto hit_time_stamp  = (std::int32_t)(result_time * 1e3);

    debug("--------------------");
    debug("Hit cellID   = {}", sim_hit.getCellID());
    debug("   position  = ({:.2f}, {:.2f}, {:.2f})", sim_hit.getPosition().x,
          sim_hit.getPosition().y, sim_hit.getPosition().z);
    debug("   xy_radius = {:.2f}", std::hypot(sim_hit.getPosition().x, sim_hit.getPosition().y));
    debug("   momentum  = ({:.2f}, {:.2f}, {:.2f})", sim_hit.getMomentum().x,
          sim_hit.getMomentum().y, sim_hit.getMomentum().z);
    debug("   edep = {:.2f}", sim_hit.getEDep());
    debug("   time = {:.4f}[ns]", sim_hit.getTime());
    debug("   particle time = {}[ns]", sim_hit.getParticle().getTime());
    debug("   time smearing: {:.4f}, resulting time = {:.4f} [ns]", time_smearing, result_time);
    debug("   hit_time_stamp: {} [~ps]", hit_time_stamp);

    if (sim_hit.getEDep() < m_cfg.threshold) {
      debug("  edep is below threshold of {:.2f} [keV]", m_cfg.threshold / dd4hep::keV);
      continue;
    }
    // the charge in RawTrackerHit is defined as int32_t to mimic ADC value.
    // because our energy threshold is often sub-keV, it's better to save charge as eV.
    // allowed range of int32_t would be eV to ~2GeV which should be sufficient.
    int32_t sim_hit_charge = sim_hit.getEDep() * dd4hep::GeV / dd4hep::eV;
    if (!cell_hit_map.contains(sim_hit.getCellID())) {
      // This cell doesn't have hits
      cell_hit_map[sim_hit.getCellID()] = {
          sim_hit.getCellID(), sim_hit_charge,
          hit_time_stamp // ns->ps
      };
    } else {
      // There is previous values in the cell
      auto& hit = cell_hit_map[sim_hit.getCellID()];
      debug("  Hit already exists in cell ID={}, prev. hit time: {}", sim_hit.getCellID(),
            hit.getTimeStamp());

      // keep earliest time for hit
      hit.setTimeStamp(std::min(hit_time_stamp, hit.getTimeStamp()));

      // sum deposited energy
      auto charge = hit.getCharge();
      hit.setCharge(charge + sim_hit_charge);
    }
  }

  for (auto item : cell_hit_map) {
    raw_hits->push_back(item.second);
    auto raw_hit = raw_hits->at(raw_hits->size() - 1);

    for (const auto& sim_hit : *sim_hits) {
      if (item.first == sim_hit.getCellID()) {
#if EDM4EIC_BUILD_VERSION >= EDM4EIC_VERSION(8, 7, 0)
        // create link
        auto link = links->create();
        link.setFrom(item.second);
        link.setTo(sim_hit);
        link.setWeight(1.0);
#endif
        // set association
        auto hitassoc = associations->create();
        double weight = 0.0;
        if (raw_hit.getCharge() > 0) {
          weight = sim_hit.getEDep() * dd4hep::GeV / dd4hep::eV / raw_hit.getCharge();
        }
        hitassoc.setWeight(weight);
        hitassoc.setRawHit(raw_hit);
        hitassoc.setSimHit(sim_hit);
      }
    }
  }
}

} // namespace eicrecon
