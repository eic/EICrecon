// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 hpDIRC GR effort

#include "DircGR.h"

#include <DD4hep/Detector.h>
#include <DD4hep/Readout.h>
#include <algorithms/geo.h>
#include <edm4eic/CherenkovParticleIDHypothesis.h>
#include <edm4eic/TrackPoint.h>
#include <fmt/ranges.h>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>

#include "algorithms/pid/DircGRCosAngTables.h"
#include "services/particle/ParticleSvc.h"

namespace eicrecon {

namespace {
  constexpr double kRadToDeg = 180.0 / M_PI;
}

void DircGR::init() {
  if (m_cfg.photonInput == "sim") {
    m_photon_input = PhotonInput::Sim;
  } else if (m_cfg.photonInput == "raw") {
    m_photon_input = PhotonInput::Raw;
  } else {
    error("photonInput must be 'sim' or 'raw', got '{}'", m_cfg.photonInput);
    throw std::runtime_error("DircGR: bad photonInput");
  }

  m_id_dec     = algorithms::GeoSvc::instance().detector()->readout(m_cfg.readout).idSpec().decoder();
  m_module_idx = m_id_dec->index("module");
  m_bar_idx    = m_id_dec->index("bar");

  // hypothesis masses
  const auto& particleSvc = algorithms::ParticleSvc::instance();
  m_hyp_mass.clear();
  for (int pdg : m_cfg.hypotheses) {
    const double m = particleSvc.particle(pdg).mass;
    m_hyp_mass.push_back(m);
    const double pthr = m / std::sqrt(m_cfg.refractiveIndex * m_cfg.refractiveIndex - 1.0);
    info("hypothesis PDG {}: mass {:.6f} GeV, Cherenkov threshold {:.3f} GeV (n={})", pdg, m,
         pthr, m_cfg.refractiveIndex);
  }

  info("photonInput={} hypotheses={} pixelSize={} mm grid {}x{} bars {}", m_cfg.photonInput,
       fmt::join(m_cfg.hypotheses, ","), m_cfg.pixelSize, m_cfg.nPixX, m_cfg.nPixY, m_cfg.nBar);

  if (m_cfg.lutPath.empty()) {
    warning("lutPath is empty: no LUT loaded, GR cannot run (skeleton/dry run only)");
  } else {
    m_lut = DircGRLUT::get(m_cfg.lutPath, m_cfg.nPixX, m_cfg.nPixY, m_cfg.nBar);
    info("LUT {}: {} paths, {:.1f} MB, loaded in {:.1f} s (shared, use_count={})", m_cfg.lutPath,
         m_lut->nPathsTotal(), m_lut->bytes() / 1048576., m_lut->loadSeconds(), m_lut.use_count());
  }
  if (m_cfg.chrcorrFile.empty()) {
    warning("chrcorrFile is empty: no chromatic correction");
  } else {
    m_chr = DircGRChrCorr::get(m_cfg.chrcorrFile, m_cfg.chrcorrSpecies, m_cfg.chrcorrMomBin);
    info("chrcorr {} (set {}_{}): apparent {}/{} pts, ideal {}/{} pts; self check vs "
         "TGraph::Eval max |diff| = {:.3g} (shared, use_count={})",
         m_cfg.chrcorrFile, m_cfg.chrcorrSpecies, m_cfg.chrcorrMomBin,
         m_chr->nPoints(DircGRChrCorr::kChrom1p0), m_chr->nPoints(DircGRChrCorr::kChrom2p0),
         m_chr->nPoints(DircGRChrCorr::kChr1p0), m_chr->nPoints(DircGRChrCorr::kChr2p0),
         m_chr->selfCheckMaxDiff(), m_chr.use_count());
    if (m_chr->selfCheckMaxDiff() > 1e-9) {
      warning("chrcorr interpolation differs from TGraph::Eval by {:.3g}",
              m_chr->selfCheckMaxDiff());
    }
  }
}

int DircGR::kzFromZ(double z) {
  int kz = static_cast<int>(std::round(z)) + dircgr::kCosAngOffset;
  if (kz < 0) {
    kz = 0;
  }
  if (kz >= dircgr::kNCosAng) {
    kz = dircgr::kNCosAng - 1;
  }
  return kz;
}

bool DircGR::computeGates(const edm4eic::TrackPoint& pt, DircGRTrackGates& g) const {
  g.box = static_cast<int>(m_id_dec->get(pt.surface, m_module_idx));
  g.bar = static_cast<int>(m_id_dec->get(pt.surface, m_bar_idx));
  if (g.box < 0 || g.box >= m_cfg.nBox || g.bar < 0 || g.bar >= m_cfg.nBar) {
    return false;
  }
  const double px = pt.momentum.x, py = pt.momentum.y, pz = pt.momentum.z;
  g.ptot          = std::sqrt(px * px + py * py + pz * pz);
  if (g.ptot <= 0) {
    return false;
  }

  // expected Cherenkov angle per hypothesis
  const double n = m_cfg.refractiveIndex;
  g.thetaCexp.assign(m_hyp_mass.size(), 0.0);
  g.thetaCexpMax = 0;
  for (std::size_t i = 0; i < m_hyp_mass.size(); i++) {
    const double beta = g.ptot / std::sqrt(g.ptot * g.ptot + m_hyp_mass[i] * m_hyp_mass[i]);
    if (beta >= 1.0 / n) {
      g.thetaCexp[i] = std::acos(1.0 / beta / n);
      g.thetaCexpMax = std::max(g.thetaCexpMax, g.thetaCexp[i]);
    }
  }

  // detector angles from the entry position, direction in the box frame
  const double x = pt.position.x, y = pt.position.y, z = pt.position.z;
  const double dist = std::sqrt(x * x + y * y + z * z);
  g.detPhi          = std::atan2(y, x);
  g.detTheta        = std::acos(z / dist);
  g.eta             = -std::log(std::tan(g.detTheta / 2.0));
  const double rot  = -g.box * (2.0 * M_PI / m_cfg.nBox);
  const double c = std::cos(rot), s = std::sin(rot);
  g.dirRot = {(c * px - s * py) / g.ptot, (s * px + c * py) / g.ptot, pz / g.ptot};
  g.zIn    = z;
  g.tIn    = pt.time;

  // earliest arrival times
  const double dZ1 = z - m_cfg.zReadout;
  const double dZ2 = (m_cfg.zBarMax - z) + (m_cfg.zBarMax - m_cfg.zReadout);
  g.tbeg1          = dZ1 / m_cfg.vBar + g.tIn;
  g.tbeg2          = dZ2 / m_cfg.vBar + g.tIn;
  double polarOP   = (g.detTheta < M_PI / 2.) ? g.detTheta - g.thetaCexpMax
                                              : g.detTheta + g.thetaCexpMax;
  polarOP = std::clamp(polarOP, 0.0, M_PI);
  g.tbeg1OP = dZ1 / std::abs(std::cos(polarOP)) / m_cfg.vBar + g.tIn;
  g.tbeg2OP = dZ2 / std::abs(std::cos(polarOP)) / m_cfg.vBar + g.tIn;
  const double thDeg = g.detTheta * kRadToDeg;
  if (thDeg > m_cfg.dir1MinThetaDeg && thDeg < m_cfg.dir2MaxThetaDeg) {
    g.tbeg1OP -= m_cfg.tbegShift90;
    g.tbeg2OP -= m_cfg.tbegShift90;
  }
  g.use1 = thDeg > m_cfg.dir1MinThetaDeg;
  g.use2 = thDeg < m_cfg.dir2MaxThetaDeg;

  // cos(angle) envelope at this entry z
  const int kz  = kzFromZ(z);
  g.cosAngUpper = dircgr::kCosAngUpper[kz];
  g.cosAngLower = dircgr::kCosAngLower[kz];

  // chromatic correction parameters at this polar angle
  g.hasChr = static_cast<bool>(m_chr);
  if (g.hasChr) {
    for (int p = 0; p < DircGRChrCorr::kNPar; p++) {
      g.chr[p] = m_chr->eval(static_cast<DircGRChrCorr::Par>(p), thDeg);
    }
  }
  return true;
}

int DircGR::timeGateCrude(double hitt, double texp1, double texp2,
                          const DircGRTrackGates& g) const {
  const double hw = m_cfg.crudeGateHalfWidth;
  const double dt1 = hitt - texp1, dt2 = hitt - texp2;
  int zdir         = -9;
  if (dt1 > -hw && dt1 < hw && g.use1) {
    zdir = -1;
  }
  if (dt2 > -hw && dt2 < hw && g.use2) {
    zdir = 1;
  }
  return zdir;
}

int DircGR::timeGateTight(double hitt, double texp1, double texp2,
                          const DircGRTrackGates& g) const {
  // half width linear in t_exp: hw0 at tbegOP, hw10 at tbegOP + 10 ns
  const double slope = (m_cfg.tightGateHalfWidth10 - m_cfg.tightGateHalfWidth0) / 10.0;
  const double cut1  = m_cfg.tightGateHalfWidth0 + slope * (texp1 - g.tbeg1OP);
  const double cut2  = m_cfg.tightGateHalfWidth0 + slope * (texp2 - g.tbeg2OP);
  const double dt1 = hitt - texp1, dt2 = hitt - texp2;
  int zdir         = -9;
  if (dt1 >= -cut1 && dt1 < cut1 && g.use1) {
    zdir = -1;
  }
  if (dt2 >= -cut2 && dt2 < cut2 && g.use2) {
    zdir = 1; // direction 2 wins if both match (as the old code)
  }
  if (zdir == -1 && hitt < g.tbeg1OP) {
    return -8;
  }
  if (zdir == 1 && hitt < g.tbeg2OP) {
    return -7;
  }
  return zdir;
}

bool DircGR::cosAngOK(double cosOPtheta, int zdir, const DircGRTrackGates& g) {
  const double c = (zdir == 1) ? -cosOPtheta : cosOPtheta;
  if (zdir != -1 && zdir != 1) {
    return false;
  }
  return c >= g.cosAngLower && c < g.cosAngUpper;
}

void DircGR::process(const Input& input, const Output& output) const {
  const auto [segments, sim_hits, raw_hits, raw_links] = input;
  auto [pids, radinfo]                                  = output;

  // photon cellIDs from the selected input (photon loop: next step)
  std::vector<std::uint64_t> photon_cells;
  if (m_photon_input == PhotonInput::Sim) {
    if (sim_hits == nullptr) {
      error("photonInput=sim but no sim photon hits provided");
      return;
    }
    for (const auto& h : *sim_hits) {
      photon_cells.push_back(h.getCellID());
    }
  } else {
    if (raw_hits == nullptr) {
      error("photonInput=raw but no raw photon hits provided");
      return;
    }
    for (const auto& h : *raw_hits) {
      photon_cells.push_back(h.getCellID());
    }
  }

  for (const auto& seg : *segments) {
    // ---- placeholder outputs, always one per segment (index-aligned) ----
    auto pid = pids->create();
    pid.setChargedParticle(seg);
    pid.setNpe(0);
    pid.setRefractiveIndex(0);
    pid.setPhotonEnergy(0);
    for (int pdg : m_cfg.hypotheses) {
      pid.addToHypotheses(edm4eic::CherenkovParticleIDHypothesis{.PDG = pdg, .npe = 0, .weight = 0});
    }
    auto ri = radinfo->create();
    ri.setNpe(0);
    ri.setNhits(0);
    ri.setAngle(0);

    const auto points = seg.getPoints();
    if (points.empty()) {
      continue;
    }
    DircGRTrackGates g;
    if (!computeGates(points[0], g)) {
      warning("segment with unusable entry point (surface {:#x}), skipped", points[0].surface);
      continue;
    }
    std::size_t nbox = 0;
    for (auto c : photon_cells) {
      if (static_cast<int>(m_id_dec->get(c, m_module_idx)) == g.box) {
        ++nbox;
      }
    }
    debug("segment box {} bar {} z_in={:.1f} p={:.3f}: {} photon hits in box", g.box, g.bar,
          g.zIn, g.ptot, nbox);
    // machine-readable dump for the gate regression check (phase3/p3gates)
    trace("GATES {} {} {:.6f} {:.6f} {:.6f} {:.6f} {:.6f} {:.6f} {:.8f} {:.8f} {:.8f} {:.8f} "
          "{:.8f} {:.8f} {:.6f} {:.6f} {:d} {:d} {:.6f} {:.6f} {:.8g} {:.8g} {:.8g} {:.8g} "
          "{:.8g} {:.8g} {:.8g} {:.8g} {:.8f}",
          g.box, g.bar, g.zIn, g.tIn, g.ptot, g.detTheta, g.detPhi, g.thetaCexpMax,
          g.thetaCexp.size() > 0 ? g.thetaCexp[0] : 0., g.thetaCexp.size() > 1 ? g.thetaCexp[1] : 0.,
          g.thetaCexp.size() > 2 ? g.thetaCexp[2] : 0., g.dirRot[0], g.dirRot[1], g.dirRot[2],
          g.tbeg1OP, g.tbeg2OP, g.use1, g.use2, g.cosAngLower, g.cosAngUpper, g.chr[0], g.chr[1],
          g.chr[2], g.chr[3], g.chr[4], g.chr[5], g.chr[6], g.chr[7], g.tbeg1);
  }
}

} // namespace eicrecon
