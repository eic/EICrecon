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
#include <podio/detail/Link.h>
#include <podio/detail/LinkCollectionImpl.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iterator>
#include <limits>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

#include "algorithms/digi/PhotoMultiplierHitDigiConfig.h"

namespace {

// clamp a value to the 32-bit integer range of the raw hit data members
template <typename T> std::int32_t clampToInt32(T value) {
  return static_cast<std::int32_t>(
      std::clamp<T>(value, static_cast<T>(std::numeric_limits<std::int32_t>::min()),
                    static_cast<T>(std::numeric_limits<std::int32_t>::max())));
}

} // namespace

namespace eicrecon {

//------------------------
// init
//------------------------
void PhotoMultiplierHitDigi::init() {
  // print the configuration parameters
  debug() << m_cfg << endmsg;

  // initialize quantum efficiency table
  qe_init();

  if (m_cfg.enableNoise && m_noisePixelCount == 0) {
    warning("Noise hits are enabled, but no noise pixels are defined; no noise hits will be added");
  }
}

//------------------------
// process
//------------------------
void PhotoMultiplierHitDigi::process(const PhotoMultiplierHitDigi::Input& input,
                                     const PhotoMultiplierHitDigi::Output& output) const {
  const auto [headers, sim_hits]     = input;
  auto [raw_hits, links, hit_assocs] = output;

  // local random generator
  auto seed = m_uid.getUniqueID(*headers, name());
  std::default_random_engine generator(seed);
  std::normal_distribution<double> gaussian;
  std::uniform_real_distribution<double> uniform;

  // collect the detected photons, with their single-photon signal amplitude
  std::vector<PhotonData> photons;
  photons.reserve(sim_hits->size());
  for (std::size_t sim_hit_index = 0; sim_hit_index < sim_hits->size(); sim_hit_index++) {
    const auto& sim_hit = sim_hits->at(sim_hit_index);
    auto edep_eV        = sim_hit.getEDep() *
                          1e9; // [GeV] -> [eV] // FIXME: use common unit converters, when available
    auto id             = sim_hit.getCellID();
    trace("hit: pixel id={:#018x}  edep = {} eV", id, edep_eV);

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
    double amp = m_cfg.speMean + gaussian(generator) * m_cfg.speError;
    photons.push_back({.cellID        = id,
                       .time          = sim_hit.getTime(),
                       .amp           = amp,
                       .sim_hit_index = sim_hit_index,
                       .is_noise      = false});
  }

  // add noise hits, uniformly distributed over the readout pixels and the noise time window
  if (m_cfg.enableNoise && m_noisePixelCount > 0) {
    // noiseRate [1/s] and noiseTimeWindow [s] are in dd4hep units
    const double mean_noise_hits =
        m_cfg.noiseRate * m_cfg.noiseTimeWindow * static_cast<double>(m_noisePixelCount);
    std::poisson_distribution<std::uint64_t> n_noise_hits_dist(mean_noise_hits);
    std::uniform_int_distribution<std::uint64_t> pixel_index_dist(0, m_noisePixelCount - 1);
    const auto n_noise_hits = n_noise_hits_dist(generator);
    trace("generating {} noise hits (mean {})", n_noise_hits, mean_noise_hits);
    for (std::uint64_t i = 0; i < n_noise_hits; i++) {
      const auto id = m_noisePixelCellID(pixel_index_dist(generator));
      const auto time =
          static_cast<TimeType>(m_cfg.noiseTimeWindow * uniform(generator) / dd4hep::ns);
      const double amp = m_cfg.speMean + gaussian(generator) * m_cfg.speError;
      photons.push_back(
          {.cellID = id, .time = time, .amp = amp, .sim_hit_index = 0, .is_noise = true});
    }
  }

  // sort photons by pixel and time, so that the grouping below does not depend on input order
  std::ranges::stable_sort(photons, [](const PhotonData& a, const PhotonData& b) {
    return std::tie(a.cellID, a.time) < std::tie(b.cellID, b.time);
  });

  // group photons in the same pixel within `hitTimeWindow` of the earliest photon of the group;
  // a photon arriving later than that starts a new group, i.e. a new raw hit in the same pixel
  std::vector<std::pair<CellIDType, HitData>> hit_groups;
  for (const auto& photon : photons) {
    if (hit_groups.empty() || hit_groups.back().first != photon.cellID ||
        photon.time - hit_groups.back().second.time > m_cfg.hitTimeWindow) {
      auto sig = photon.amp + m_cfg.pedMean + m_cfg.pedError * gaussian(generator);
      hit_groups.emplace_back(
          photon.cellID,
          HitData{.npe = 1, .signal = sig, .time = photon.time, .sim_hit_indices = {}});
    } else {
      auto& group = hit_groups.back().second;
      group.npe += 1;
      group.signal += photon.amp;
    }
    if (!photon.is_noise) {
      hit_groups.back().second.sim_hit_indices.push_back(photon.sim_hit_index);
    }
  }

  // print `hit_groups`
  if (level() <= algorithms::LogLevel::kTrace) {
    for (const auto& [id, hit] : hit_groups) {
      trace("hit_group: pixel id={:#018x} -> npe={} signal={} time={}", id, hit.npe, hit.signal,
            hit.time);
      for (auto i : hit.sim_hit_indices) {
        trace(" - MC hit: EDep={}, id={}", sim_hits->at(i).getEDep(),
              sim_hits->at(i).getObjectID().index);
      }
    }
  }

  // build output `RawTrackerHit` and `MCRecoTrackerHitAssociation` collections
  trace("{:-<70}", "Digitized raw hits ");
  for (const auto& [id, data] : hit_groups) {

    // build `RawTrackerHit`
    auto raw_hit = raw_hits->create();
    raw_hit.setCellID(id);
    raw_hit.setCharge(clampToInt32(std::llround(data.signal)));
    // TDC counts: index of the time bin of width `timeResolution`
    raw_hit.setTimeStamp(clampToInt32(std::floor(data.time / m_cfg.timeResolution)));
    trace("raw_hit: cellID={:#018x} -> charge={} timeStamp={}", raw_hit.getCellID(),
          raw_hit.getCharge(), raw_hit.getTimeStamp());

    // build `MCRecoTrackerHitAssociation` (for non-noise hits only)
    for (auto i : data.sim_hit_indices) {
      // create link
      auto link = links->create();
      link.setFrom(raw_hit);
      link.setTo(sim_hits->at(i));
      link.setWeight(1.0 / data.sim_hit_indices.size());
      auto hit_assoc = hit_assocs->create();
      hit_assoc.setWeight(1.0 / data.sim_hit_indices.size()); // not used
      hit_assoc.setRawHit(raw_hit);
      hit_assoc.setSimHit(sim_hits->at(i));
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

} // namespace eicrecon
