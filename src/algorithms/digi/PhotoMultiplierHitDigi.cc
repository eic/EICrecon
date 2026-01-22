// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023, Chao Peng, Thomas Britton, Christopher Dilks, Luigi Dello Stritto

/*  General PhotoMultiplier Digitization
 *
 *  Apply the given quantum efficiency for photon detection
 *  Converts the number of detected photons to signal amplitude
 *
 *  Author: Chao Peng (ANL)
 *  Date: 10/02/2020
 *
 *  Ported from Juggler by Thomas Britton (JLab)
 */

#include "PhotoMultiplierHitDigi.h"

#include <Evaluator/DD4hepUnits.h>
#include <algorithms/logger.h>
#include <edm4hep/Vector3d.h>
#include <podio/ObjectID.h>
#include <algorithm>
#include <cmath>
#include <gsl/pointers>
#include <iterator>

#include "algorithms/digi/PhotoMultiplierHitDigiConfig.h"

namespace eicrecon {

//------------------------
// init
//------------------------
void PhotoMultiplierHitDigi::init() {
  // print the configuration parameters
  debug() << m_cfg << endmsg;

  // initialize quantum efficiency table
  qe_init();
}

//------------------------
// process
//------------------------
void PhotoMultiplierHitDigi::process(const PhotoMultiplierHitDigi::Input& input,
                                     const PhotoMultiplierHitDigi::Output& output) const {
  const auto [headers, sim_hits] = input;
  auto [raw_hits, hit_assocs]    = output;

  // local random generator
  auto seed = m_uid.getUniqueID(*headers, name());
  std::default_random_engine generator(seed);
  std::normal_distribution<double> gaussian;
  std::uniform_real_distribution<double> uniform;

  trace("{:=^70}", " call PhotoMultiplierHitDigi::process ");
  std::map<CellIDType, std::vector<HitData>> hit_groups;
  // collect the photon hit in the same cell
  // calculate signal
  trace("{:-<70}", "Loop over simulated hits ");
  for (std::size_t sim_hit_index = 0; sim_hit_index < sim_hits->size(); sim_hit_index++) {
    const auto& sim_hit = sim_hits->at(sim_hit_index);
    auto edep_eV        = sim_hit.getEDep() *
                   1e9; // [GeV] -> [eV] // FIXME: use common unit converters, when available
    auto id = sim_hit.getCellID();
    trace("hit: pixel id={:#018X}  edep = {} eV", id, edep_eV);

    // overall safety factor
    if (uniform(generator) > m_cfg.safetyFactor) {
      continue;
    }

    // quantum efficiency
    if (m_cfg.enableQuantumEfficiency and !qe_pass(edep_eV, uniform(generator))) {
      continue;
    }

    // pixel gap cuts
    if (m_cfg.enablePixelGaps) {
      auto pos = sim_hit.getPosition();
      if (!m_PixelGapMask(
              id, dd4hep::Position(pos.x * dd4hep::mm, pos.y * dd4hep::mm, pos.z * dd4hep::mm))) {
        continue;
      }
    }

    // cell time, signal amplitude, truth photon
    trace(" -> hit accepted");
    trace(" -> MC hit id={}", sim_hit.getObjectID().index);
    auto time  = sim_hit.getTime();
    double amp = m_cfg.speMean + gaussian(generator) * m_cfg.speError;

    // insert hit to `hit_groups`
    InsertHit(hit_groups, id, amp, time, sim_hit_index, generator, gaussian);
  }

  // print `hit_groups`
  if (level() <= algorithms::LogLevel::kTrace) {
    trace("{:-<70}", "Accepted hit groups ");
    for (auto& [id, hitVec] : hit_groups) {
      for (auto& hit : hitVec) {
        trace("hit_group: pixel id={:#018X} -> npe={} signal={} time={}", id, hit.npe, hit.signal,
              hit.time);
        for (auto i : hit.sim_hit_indices) {
          trace(" - MC hit: EDep={}, id={}", sim_hits->at(i).getEDep(),
                sim_hits->at(i).getObjectID().index);
        }
      }
    }
  }

  //build noise raw hits
  if (m_cfg.enableNoise) {
    trace("{:=^70}", " BEGIN NOISE INJECTION ");
    float p            = m_cfg.noiseRate * m_cfg.noiseTimeWindow;
    auto cellID_action = [this, &gaussian, &generator, &hit_groups, &uniform](auto id) {
      // cell time, signal amplitude
      double amp    = m_cfg.speMean + gaussian(generator) * m_cfg.speError;
      TimeType time = m_cfg.noiseTimeWindow * uniform(generator) / dd4hep::ns;

      // insert in `hit_groups`, or if the pixel already has a hit, update `npe` and `signal`
      this->InsertHit(hit_groups, id, amp, time,
                      0, // not used
                      generator, gaussian, true);
    };
    m_VisitRngCellIDs(cellID_action, p);
  }

  // build output `RawTrackerHit` and `MCRecoTrackerHitAssociation` collections
  trace("{:-<70}", "Digitized raw hits ");
  for (auto& it : hit_groups) {
    for (auto& data : it.second) {

      // build `RawTrackerHit`
      auto raw_hit = raw_hits->create();
      raw_hit.setCellID(it.first);
      raw_hit.setCharge(static_cast<decltype(edm4eic::RawTrackerHitData::charge)>(data.signal));
      raw_hit.setTimeStamp(static_cast<decltype(edm4eic::RawTrackerHitData::timeStamp)>(
          data.time / m_cfg.timeResolution));
      trace("raw_hit: cellID={:#018X} -> charge={} timeStamp={}", raw_hit.getCellID(),
            raw_hit.getCharge(), raw_hit.getTimeStamp());

      // build `MCRecoTrackerHitAssociation` (for non-noise hits only)
      if (!data.sim_hit_indices.empty()) {
        for (auto i : data.sim_hit_indices) {
          auto hit_assoc = hit_assocs->create();
          hit_assoc.setWeight(1.0 / data.sim_hit_indices.size()); // not used
          hit_assoc.setRawHit(raw_hit);
          hit_assoc.setSimHit(sim_hits->at(i));
        }
      }
    }
  }
}

void PhotoMultiplierHitDigi::qe_init() {
  // get quantum efficiency table
  qeff.clear();
  auto hc = dd4hep::h_Planck * dd4hep::c_light / (dd4hep::eV * dd4hep::nm); // [eV*nm]
  for (const auto& [wl, qe] : m_cfg.quantumEfficiency) {
    qeff.emplace_back(hc / wl, qe); // convert wavelength [nm] -> energy [eV]
  }

  // sort quantum efficiency data first
  std::ranges::sort(qeff, [](const std::pair<double, double>& v1,
                             const std::pair<double, double>& v2) { return v1.first < v2.first; });

  // print the table
  debug("{:-^60}", " Quantum Efficiency vs. Energy ");
  for (auto& [en, qe] : qeff) {
    debug("  {:>10.4} {:<}", en, qe);
  }
  trace("{:=^60}", "");

  if (m_cfg.enableQuantumEfficiency) {
    // sanity checks
    if (qeff.empty()) {
      qeff = {{2.6, 0.3}, {7.0, 0.3}};
      warning("Invalid quantum efficiency data provided, using default values {} {:.2f} {} {:.2f} "
              "{} {:.2f} {} {:.2f} {}",
              "{{", qeff.front().first, ",", qeff.front().second, "},{", qeff.back().first, ",",
              qeff.back().second, "}}");
    }
    if (qeff.front().first > 3.0) {
      warning("Quantum efficiency data start from {:.2f} {}", qeff.front().first,
              " eV, maybe you are using wrong units?");
    }
    if (qeff.back().first < 3.0) {
      warning("Quantum efficiency data end at {:.2f} {}", qeff.back().first,
              " eV, maybe you are using wrong units?");
    }
  }
}

template <class RndmIter, typename T, class Compare>
RndmIter PhotoMultiplierHitDigi::interval_search(RndmIter beg, RndmIter end, const T& val,
                                                 Compare comp) const {
  // special cases
  auto dist = std::distance(beg, end);
  if ((dist < 2) || (comp(*beg, val) > 0) || (comp(*std::prev(end), val) < 0)) {
    return end;
  }
  auto mid = std::next(beg, dist / 2);

  while (mid != end) {
    if (comp(*mid, val) == 0) {
      return mid;
    }
    if (comp(*mid, val) > 0) {
      end = mid;
    } else {
      beg = std::next(mid);
    }
    mid = std::next(beg, std::distance(beg, end) / 2);
  }

  if (mid == end || comp(*mid, val) > 0) {
    return std::prev(mid);
  }
  return mid;
}

bool PhotoMultiplierHitDigi::qe_pass(double ev, double rand) const {
  auto it = interval_search(
      qeff.begin(), qeff.end(), ev,
      [](const std::pair<double, double>& vals, double val) { return vals.first - val; });

  if (it == qeff.end()) {
    // warning("{} eV is out of QE data range, assuming 0\% efficiency",ev);
    return false;
  }

  double prob = it->second;
  auto itn    = std::next(it);
  if (itn != qeff.end() && (itn->first - it->first != 0)) {
    prob = (it->second * (itn->first - ev) + itn->second * (ev - it->first)) /
           (itn->first - it->first);
  }

  // trace("{} eV, QE: {}\%",ev,prob*100.);
  return rand <= prob;
}

// add a hit to local `hit_groups` data structure
// NOLINTBEGIN(bugprone-easily-swappable-parameters)
void PhotoMultiplierHitDigi::InsertHit(
    std::map<CellIDType, std::vector<HitData>>& hit_groups, CellIDType id, double amp,
    TimeType time, std::size_t sim_hit_index, std::default_random_engine& generator,
    std::normal_distribution<double>& gaussian,
    bool is_noise_hit) const // NOLINTEND(bugprone-easily-swappable-parameters)
{
  auto it = hit_groups.find(id);
  if (it != hit_groups.end()) {
    std::size_t i = 0;
    for (auto ghit = it->second.begin(); ghit != it->second.end(); ++ghit, ++i) {
      if (std::abs(time - ghit->time) <= (m_cfg.hitTimeWindow)) {
        // hit group found, update npe, signal, and list of MC hits
        ghit->npe += 1;
        ghit->signal += amp;
        if (!is_noise_hit) {
          ghit->sim_hit_indices.push_back(sim_hit_index);
        }
        trace(" -> add to group @ {:#018X}: signal={}", id, ghit->signal);
        break;
      }
    }
    // no hits group found
    if (i >= it->second.size()) {
      auto sig = amp + m_cfg.pedMean + m_cfg.pedError * gaussian(generator);
      decltype(HitData::sim_hit_indices) indices;
      if (!is_noise_hit) {
        indices.push_back(sim_hit_index);
      }
      hit_groups.insert(
          {id, {HitData{.npe = 1, .signal = sig, .time = time, .sim_hit_indices = indices}}});
      trace(" -> no group found,");
      trace("    so new group @ {:#018X}: signal={}", id, sig);
    }
  } else {
    auto sig = amp + m_cfg.pedMean + m_cfg.pedError * gaussian(generator);
    decltype(HitData::sim_hit_indices) indices;
    if (!is_noise_hit) {
      indices.push_back(sim_hit_index);
    }
    hit_groups.insert(
        {id, {HitData{.npe = 1, .signal = sig, .time = time, .sim_hit_indices = indices}}});
    trace(" -> new group @ {:#018X}: signal={}", id, sig);
  }
}

} // namespace eicrecon
