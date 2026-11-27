// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 W.J. Llope

#include "DircGRdig.h"

#include <edm4eic/unit_system.h>
#include <podio/detail/Link.h>
#include <podio/detail/LinkCollectionImpl.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <random>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

#include "algorithms/digi/DircGRdigConfig.h"

namespace eicrecon {

void DircGRdig::init() {
  if (m_cfg.timeResolution <= 0) {
    throw std::runtime_error("DircGRdig: timeResolution must be > 0");
  }
  debug("deadTime {} ns, timeResolution (TDC LSB) {} ns, timeJitter {} ns",
        m_cfg.deadTime / edm4eic::unit::ns, m_cfg.timeResolution / edm4eic::unit::ns,
        m_cfg.timeJitter / edm4eic::unit::ns);
}

void DircGRdig::process(const Input& input, const Output& output) const {
  const auto [headers, sim_hits] = input;
  auto [raw_hits, links]         = output;

  std::default_random_engine generator(m_uid.getUniqueID(*headers, name()));
  std::normal_distribution<double> gaussian(0.0, 1.0);

  // photons per pixel (measured time, sim hit index); pixels ordered by
  // cellID (deterministic output order), photons kept in time order (equal
  // times in input order)
  struct Photon {
    double t;
    std::size_t index;
  };
  std::map<std::uint64_t, std::vector<Photon>> pixels;
  for (std::size_t i = 0; i < sim_hits->size(); i++) {
    const auto& h = sim_hits->at(i);
    double t      = h.getTime();
    if (m_cfg.timeJitter > 0) {
      t += m_cfg.timeJitter * gaussian(generator);
    }
    auto& photons = pixels[h.getCellID()];
    photons.insert(std::ranges::upper_bound(photons, t, {}, &Photon::t), {t, i});
  }

  for (const auto& [id, photons] : pixels) {
    std::size_t first = 0;
    while (first < photons.size()) {
      // hit = photons [first, last) within the dead time of photons[first]
      std::size_t last = first + 1;
      while (last < photons.size() && photons[last].t - photons[first].t < m_cfg.deadTime) {
        ++last;
      }
      const auto n = last - first;
      auto raw     = raw_hits->create();
      raw.setCellID(id);
      raw.setCharge(static_cast<std::int32_t>(n));
      raw.setTimeStamp(
          static_cast<std::int32_t>(std::floor(photons[first].t / m_cfg.timeResolution)));
      for (std::size_t k = first; k < last; k++) {
        const auto& sim = sim_hits->at(photons[k].index);
        auto link       = links->create();
        link.setFrom(raw);
        link.setTo(sim);
        link.setWeight(1.0 / n);
      }
      trace("pixel {:#018x}: hit t={:.4f} ns ({} photons)", id, photons[first].t, n);
      first = last;
    }
  }
  debug("{} photons -> {} raw hits in {} pixels", sim_hits->size(), raw_hits->size(),
        pixels.size());
}

} // namespace eicrecon
