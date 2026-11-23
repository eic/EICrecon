// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 hpDIRC GR effort

#include "DircTruthIncidence.h"

#include <edm4eic/TrackPoint.h>
#include <edm4hep/MCParticle.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/utils/vector_utils.h>
#include <cmath>
#include <map>

namespace eicrecon {

namespace {
  constexpr double kC = 299.792458; // [mm/ns]
}

void DircTruthIncidence::init() {
  debug("refractiveIndex={} betaMargin={} primaryOnly={} dropBornInside={} tol={} mm",
        m_cfg.refractiveIndex, m_cfg.betaMargin, m_cfg.primaryOnly, m_cfg.dropBornInside,
        m_cfg.bornInsideTolerance);
}

void DircTruthIncidence::process(const Input& input, const Output& output) const {
  const auto [in_hits]          = input;
  auto [out_segments, out_hits] = output;
  out_hits->setSubsetCollection();

  // earliest quartz hit per MCParticle; keyed by MCParticle index so the
  // output order is deterministic
  std::map<int, edm4hep::SimTrackerHit> first_hit;
  for (const auto& hit : *in_hits) {
    if (hit.isProducedBySecondary()) {
      continue;
    }
    const auto mcp = hit.getParticle();
    if (!mcp.isAvailable()) {
      continue;
    }
    if (mcp.getPDG() == -22 || mcp.getCharge() == 0) {
      continue; // absorbed optical photons, neutrals
    }
    if (m_cfg.primaryOnly && mcp.getGeneratorStatus() != 1) {
      continue;
    }
    const int key = mcp.getObjectID().index;
    auto it       = first_hit.find(key);
    if (it == first_hit.end() || hit.getTime() < it->second.getTime()) {
      first_hit.insert_or_assign(key, hit);
    }
  }

  const double beta_min = 1.0 / m_cfg.refractiveIndex + m_cfg.betaMargin;

  for (const auto& [key, hit] : first_hit) {
    const auto mcp  = hit.getParticle();
    const double m  = mcp.getMass();
    auto pv         = hit.getMomentum();
    double p        = edm4hep::utils::magnitude(pv);
    const double L  = hit.getPathLength();
    const auto xmid = hit.getPosition();
    if (p <= 0) {
      continue;
    }
    const double ux = pv.x / p, uy = pv.y / p, uz = pv.z / p;

    // particle destroyed in the bar: hit momentum = (p_entry + 0)/2
    if (m_cfg.fixEndsInside) {
      const auto e    = mcp.getEndpoint();
      const double dx = e.x - (xmid.x + 0.5 * L * ux);
      const double dy = e.y - (xmid.y + 0.5 * L * uy);
      const double dz = e.z - (xmid.z + 0.5 * L * uz);
      if (std::sqrt(dx * dx + dy * dy + dz * dz) < m_cfg.endsInsideTolerance) {
        pv = 2.0f * pv;
        p *= 2.0;
        debug("PDG {} ends inside bar (L={:.1f} mm): p_hit doubled to {:.4f}", mcp.getPDG(), L, p);
      }
    }

    // midpoint beta (for the time correction, as validated in Phase 1) and an
    // entry estimate for the threshold cut, p_in = p_mid + dp/2 with
    // dp = eDep/beta (dE = beta dp): protons near threshold otherwise drop
    // out (Phase 1/2 S2p finding)
    const double beta    = p / std::sqrt(p * p + m * m);
    const double p_in    = p + 0.5 * hit.getEDep() / beta;
    const double beta_in = p_in / std::sqrt(p_in * p_in + m * m);
    if (beta_in <= beta_min) {
      trace("PDG {} p={:.3f} beta_in={:.4f} below threshold", mcp.getPDG(), p, beta_in);
      continue;
    }

    if (m_cfg.dropBornInside) {
      const auto v    = mcp.getVertex();
      const double dx = v.x - xmid.x, dy = v.y - xmid.y, dz = v.z - xmid.z;
      if (std::sqrt(dx * dx + dy * dy + dz * dz) < 0.5 * L + m_cfg.bornInsideTolerance) {
        trace("PDG {} born inside bar, skipped", mcp.getPDG());
        continue;
      }
    }

    const edm4hep::Vector3f x_in{static_cast<float>(xmid.x - 0.5 * L * ux),
                                 static_cast<float>(xmid.y - 0.5 * L * uy),
                                 static_cast<float>(xmid.z - 0.5 * L * uz)};
    const float t_in = hit.getTime() - 0.25 * L / (beta * kC);

    edm4eic::TrackPoint point{
        .surface    = hit.getCellID(),
        .system     = 0,
        .position   = x_in,
        .momentum   = pv,
        .time       = t_in,
        .theta      = static_cast<float>(edm4hep::utils::anglePolar(pv)),
        .phi        = static_cast<float>(edm4hep::utils::angleAzimuthal(pv)),
        .pathlength = 0, // not known at truth level
    };
    auto seg = out_segments->create();
    seg.setLength(L);
    seg.addToPoints(point);
    out_hits->push_back(hit);

    debug("PDG {:6d} gst {} p={:7.3f} x_in=({:8.2f},{:8.2f},{:8.2f}) t_in={:.4f} cell={:#x}",
          mcp.getPDG(), mcp.getGeneratorStatus(), p, x_in.x, x_in.y, x_in.z, t_in, hit.getCellID());
  }
}

} // namespace eicrecon
