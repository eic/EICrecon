// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Whitney Armstrong, Wouter Deconinck, Sylvester Joosten, Dmitry Romanov

#include <Evaluator/DD4hepUnits.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/Vector3d.h>
#include <edm4hep/Vector3f.h>
#include <podio/ObjectID.h>
#include <podio/detail/Link.h>
#include <podio/detail/LinkCollectionImpl.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <random>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "SiliconTrackerDigi.h"
#include "algorithms/digi/SiliconTrackerDigiConfig.h"

namespace eicrecon {

void SiliconTrackerDigi::init() {}

void SiliconTrackerDigi::process(const SiliconTrackerDigi::Input& input,
                                 const SiliconTrackerDigi::Output& output) const {

  const auto [headers, sim_hits]       = input;
  auto [raw_hits, links, associations] = output;

  // local random generator
  auto seed = m_uid.getUniqueID(*headers, name());
  std::default_random_engine generator(seed);
  std::normal_distribution<double> gaussian;

  // A map of unique cellIDs with temporary structure RawHit
  std::unordered_map<std::uint64_t, edm4eic::MutableRawTrackerHit> cell_hit_map;

  struct Contributor {
    std::size_t simHitIndex = 0;
    double eDep             = 0.0;
  };
  // Keep exactly the SimTrackerHits that contributed to the digitized charge.
  // Re-scanning all input SimHits after thresholding used to associate subthreshold
  // deposits to an otherwise fired cell, making truth weights inconsistent with
  // the RawTrackerHit charge itself.
  std::unordered_map<std::uint64_t, std::vector<Contributor>> cell_contributors;

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

    cell_contributors[sim_hit.getCellID()].push_back(
        {static_cast<std::size_t>(sim_hit.id().index), sim_hit.getEDep()});

    if (!cell_hit_map.contains(sim_hit.getCellID())) {
      // This cell doesn't have hits
      cell_hit_map[sim_hit.getCellID()] = {
          sim_hit.getCellID(), (std::int32_t)std::llround(sim_hit.getEDep() * 1e6),
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
      hit.setCharge(charge + (std::int32_t)std::llround(sim_hit.getEDep() * 1e6));
    }
  }

  for (const auto& [cell_id, cell_hit] : cell_hit_map) {
    raw_hits->push_back(cell_hit);
    auto raw_hit = raw_hits->at(raw_hits->size() - 1);

    const auto contributor_it = cell_contributors.find(cell_id);
    if (contributor_it == cell_contributors.end()) {
      continue;
    }
    const auto& contributors = contributor_it->second;
    double total_edep        = 0.0;
    for (const auto& contributor : contributors) {
      total_edep += contributor.eDep;
    }

    for (const auto& contributor : contributors) {
      const auto sim_hit = sim_hits->at(contributor.simHitIndex);
      const double weight =
          total_edep > 0.0 ? contributor.eDep / total_edep : 1.0 / contributors.size();

      // Link weights now describe the fraction of this RawTrackerHit's digitized
      // charge contributed by the SimTrackerHit.
      auto link = links->create();
      link.setFrom(raw_hit);
      link.setTo(sim_hit);
      link.setWeight(weight);

      auto hitassoc = associations->create();
      hitassoc.setWeight(weight);
      hitassoc.setRawHit(raw_hit);
      hitassoc.setSimHit(sim_hit);
    }
  }
}

} // namespace eicrecon
