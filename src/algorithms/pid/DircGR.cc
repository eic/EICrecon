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
  m_id_dec = algorithms::GeoSvc::instance().detector()->readout(m_cfg.readout).idSpec().decoder();
  m_module_idx = m_id_dec->index("module");
  m_bar_idx    = m_id_dec->index("bar");

  // hypothesis masses
  const auto& particleSvc = algorithms::ParticleSvc::instance();
  m_hyp_mass.clear();
  for (int pdg : m_cfg.hypotheses) {
    const double m = particleSvc.particle(pdg).mass;
    m_hyp_mass.push_back(m);
    const double pthr = m / std::sqrt(m_cfg.refractiveIndex * m_cfg.refractiveIndex - 1.0);
    info("hypothesis PDG {}: mass {:.6f} GeV, Cherenkov threshold {:.3f} GeV (n={})", pdg, m, pthr,
         m_cfg.refractiveIndex);
  }

  info("hypotheses={} internal pixel grid {}x{} of {} mm (must match the LUT), bars {}, "
       "truth digitization",
       fmt::join(m_cfg.hypotheses, ","), m_cfg.nPixX, m_cfg.nPixY, m_cfg.pixelSize, m_cfg.nBar);
  info("likelihood: spr={} mrad sigmaExt={} mrad minPhotons={}", m_cfg.spr, m_cfg.sigmaExt,
       m_cfg.minPhotons);

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
  g.ptot = std::sqrt(px * px + py * py + pz * pz);
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
  double polarOP =
      (g.detTheta < M_PI / 2.) ? g.detTheta - g.thetaCexpMax : g.detTheta + g.thetaCexpMax;
  polarOP            = std::clamp(polarOP, 0.0, M_PI);
  g.tbeg1OP          = dZ1 / std::abs(std::cos(polarOP)) / m_cfg.vBar + g.tIn;
  g.tbeg2OP          = dZ2 / std::abs(std::cos(polarOP)) / m_cfg.vBar + g.tIn;
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

int DircGR::timeGateTight(double hitt, double texp1, double texp2,
                          const DircGRTrackGates& g) const {
  // half width linear in t_exp: hw0 at tbegOP, hw10 at tbegOP + 10 ns
  const double slope = (m_cfg.tightGateHalfWidth10 - m_cfg.tightGateHalfWidth0) / 10.0;
  const double cut1  = m_cfg.tightGateHalfWidth0 + slope * (texp1 - g.tbeg1OP);
  const double cut2  = m_cfg.tightGateHalfWidth0 + slope * (texp2 - g.tbeg2OP);
  const double dt1 = hitt - texp1, dt2 = hitt - texp2;
  int zdir = -9;
  if (dt1 >= -cut1 && dt1 < cut1 && g.use1) {
    zdir = -1;
  }
  if (dt2 >= -cut2 && dt2 < cut2 && g.use2) {
    zdir = 1; // direction 2 wins if both match
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

void DircGR::photonLoop(const DircGRTrackGates& g, const std::vector<PixelHit>& hits,
                        DircGRTrackResult& r) const {
  const std::size_t nh = g.thetaCexp.size();
  r                    = DircGRTrackResult{};
  r.nBestHyp.assign(nh, 0);
  const int nbins   = m_cfg.thetaCHistBins;
  const double hmax = m_cfg.thetaCHistMax;
  std::vector<double> hist(nbins, 0.0);
  double sum = 0;

  const double dZbar1 = g.zIn - m_cfg.zBarMin;
  const double dZbar2 = (m_cfg.zBarMax - g.zIn) + (m_cfg.zBarMax - m_cfg.zBarMin);
  const auto& u       = g.dirRot;
  std::vector<double> diffbest(nh), thbest(nh);
  std::vector<int> ambbest(nh);
  // azimuth reference around the track: e1 = bar axis (box-frame z)
  // projected perpendicular to the track, e2 = u x e1
  std::array<double, 3> e1{-u[2] * u[0], -u[2] * u[1], 1.0 - u[2] * u[2]};
  {
    const double n1 = std::sqrt(e1[0] * e1[0] + e1[1] * e1[1] + e1[2] * e1[2]);
    for (auto& v : e1) {
      v /= n1;
    }
  }
  const std::array<double, 3> e2{u[1] * e1[2] - u[2] * e1[1], u[2] * e1[0] - u[0] * e1[2],
                                 u[0] * e1[1] - u[1] * e1[0]};

  for (const auto& h : hits) {
    if (h.box != g.box) {
      continue;
    }
    ++r.nHitsBox;
    if (h.ix < 0) {
      continue;
    }
    ++r.nHitsPix;
    const int ix = h.ix, iy = h.iy;
    if (!m_lut) {
      continue;
    }
    const auto P = m_lut->paths(ix, iy, g.bar);
    bool used    = false;
    for (std::size_t ip = 0; ip < P.size(); ip++) {
      const double dx = P.dirx[ip], dy = P.diry[ip], dz = P.dirz[ip];
      const double OPtheta = std::acos(dz / std::sqrt(dx * dx + dy * dy + dz * dz));
      const double acos_th = std::abs(std::cos(OPtheta));
      const double texp1   = P.time[ip] + dZbar1 / acos_th / m_cfg.vBar + g.tIn;
      const double texp2   = P.time[ip] + dZbar2 / acos_th / m_cfg.vBar + g.tIn;

      // ---- time gate (also resolves the z ambiguity), polar-angle gate ----
      const int zdir = timeGateTight(h.t, texp1, texp2, g);
      if (zdir != -1 && zdir != 1) {
        continue;
      }
      ++r.nPairsTight;
      if (!cosAngOK(std::cos(OPtheta), zdir, g)) {
        continue;
      }
      ++r.nPairsGated;

      // ---- chromatic correction ("apparent": pol1 in dt / path length) ----
      double chrcorr = 0;
      if (g.hasChr) {
        const double dt    = h.t - ((zdir == -1) ? texp1 : texp2);
        const double Lpath = P.time[ip] * m_cfg.vBar + ((zdir == -1) ? dZbar1 : dZbar2) / acos_th;
        chrcorr =
            (zdir == -1)
                ? g.chr[DircGRChrCorr::kChrom1p0] + g.chr[DircGRChrCorr::kChrom1p1] * dt / Lpath
                : g.chr[DircGRChrCorr::kChrom2p0] + g.chr[DircGRChrCorr::kChrom2p1] * dt / Lpath;
      }

      // ---- 8-fold direction ambiguity: flip x (bit 0), y (bit 1), z (bit
      // 2); the time gate fixes z: zdir -1 -> no z flip, +1 -> z flip ----
      std::fill(diffbest.begin(), diffbest.end(), 99999.);
      std::fill(thbest.begin(), thbest.end(), 0.);
      std::fill(ambbest.begin(), ambbest.end(), -1);
      const int a0 = (zdir == -1) ? 0 : 4;
      for (int iamb = a0; iamb < a0 + 4; iamb++) {
        const double sx = (iamb & 1) ? -1 : 1, sy = (iamb & 2) ? -1 : 1, sz = (iamb & 4) ? -1 : 1;
        const double th = std::acos(sx * dx * u[0] + sy * dy * u[1] + sz * dz * u[2]) - chrcorr;
        for (std::size_t ih = 0; ih < nh; ih++) {
          const double d = std::abs(th - g.thetaCexp[ih]);
          if (d < diffbest[ih]) {
            diffbest[ih] = d;
            thbest[ih]   = th;
            ambbest[ih]  = iamb;
          }
        }
      }
      // best hypothesis for this photon path
      int ibest    = -1;
      double dbest = 999;
      for (std::size_t ih = 0; ih < nh; ih++) {
        if (diffbest[ih] < dbest) {
          ibest = static_cast<int>(ih);
          dbest = diffbest[ih];
        }
      }
      const double thsel = (ibest >= 0) ? 1000. * thbest[ibest] : 0.; // [mrad]
      if (ibest >= 0) {
        ++r.nBestHyp[ibest];
        // per-path (theta, phi) [rad] of the selected ambiguity
        const int a     = ambbest[ibest];
        const double vx = ((a & 1) ? -dx : dx), vy = ((a & 2) ? -dy : dy),
                     vz = ((a & 4) ? -dz : dz);
        const double phi =
            std::atan2(vx * e2[0] + vy * e2[1] + vz * e2[2], vx * e1[0] + vy * e1[1] + vz * e1[2]);
        r.thetaPhi.emplace_back(thbest[ibest], phi);
      } else {
        r.thetaPhi.emplace_back(0., 0.);
      }
      if (thsel >= 0 && thsel < hmax) { // NaN and out-of-range dropped
        ++hist[static_cast<int>(thsel * nbins / hmax)];
      }
      sum += thsel;
      used = true;
    }
    if (used) {
      ++r.nPhotonsUsed;
    }
  }

  // ---- per-track estimators ----
  r.thetaCMean = (r.nPairsGated > 0) ? sum / r.nPairsGated : 0.;
  // track thetaC: mean of the histogram within +-peakHalfWindow bins of
  // its (first) maximum bin; bins numbered 1..nbins
  const int kmax = static_cast<int>(std::max_element(hist.begin(), hist.end()) - hist.begin()) + 1;
  const int klo  = std::max(1, kmax - m_cfg.peakHalfWindow);
  const int khi  = std::min(nbins, kmax + m_cfg.peakHalfWindow);
  double nx = 0, mx = 0;
  const double w = hmax / nbins;
  for (int kb = klo; kb <= khi; kb++) {
    nx += hist[kb - 1];
    mx += hist[kb - 1] * (kb - 0.5) * w;
  }
  r.thetaCPeak = (nx > 0) ? mx / nx : 0.;
}

std::vector<DircGR::PixelHit> DircGR::digitize(const edm4hep::SimTrackerHitCollection& hits) const {
  std::vector<PixelHit> out;
  out.reserve(hits.size());
  for (const auto& h : hits) {
    const int box = static_cast<int>(m_id_dec->get(h.getCellID(), m_module_idx));
    // box-local readout-plane coordinates: rotate box `box` onto box 0,
    // locx = -y', locy = x' (convention of the LUT generation)
    const double rot = -box * (2.0 * M_PI / m_cfg.nBox);
    const double c = std::cos(rot), s = std::sin(rot);
    const auto& x   = h.getPosition();
    const double xr = c * x.x - s * x.y, yr = s * x.x + c * x.y;
    const double locx = -yr, locy = xr;
    int ix = static_cast<int>(std::floor((locx - m_cfg.locXmin) / m_cfg.pixelSize));
    int iy = static_cast<int>(std::floor((locy - m_cfg.locYmin) / m_cfg.pixelSize));
    if (ix < 0 || ix >= m_cfg.nPixX || iy < 0 || iy >= m_cfg.nPixY) {
      ix = iy = -1;
    }
    // truth digitization: exact time, every photon is a hit
    out.push_back({box, ix, iy, h.getTime()});
  }
  return out;
}

void DircGR::process(const Input& input, const Output& output) const {
  const auto [segments, sim_hits] = input;
  auto [pids, radinfo]            = output;

  const auto photons = digitize(*sim_hits);

  for (const auto& seg : *segments) {
    // always one PID + RadiatorInfo per segment (index-aligned)
    auto pid = pids->create();
    auto ri  = radinfo->create();
    pid.setChargedParticle(seg);
    pid.setRefractiveIndex(static_cast<float>(m_cfg.refractiveIndex));
    pid.setPhotonEnergy(0); // not measured

    DircGRTrackGates g;
    DircGRTrackResult r;
    const auto points = seg.getPoints();
    bool ok           = !points.empty() && computeGates(points[0], g);
    if (!ok) {
      warning("segment without usable entry point, no PID");
    } else {
      photonLoop(g, photons, r);
    }
    const bool hasPID = ok && m_lut && r.nPhotonsUsed >= m_cfg.minPhotons;

    // ---- CherenkovParticleID ----
    const double sigma =
        hasPID ? std::sqrt(m_cfg.spr * m_cfg.spr / r.nPhotonsUsed + m_cfg.sigmaExt * m_cfg.sigmaExt)
               : 0.; // [mrad]
    pid.setNpe(hasPID ? static_cast<float>(r.nPhotonsUsed) : 0.f);
    for (std::size_t ih = 0; ih < m_cfg.hypotheses.size(); ih++) {
      float w = 0, npe = 0;
      if (hasPID) {
        const double d = (r.thetaCPeak - 1000. * g.thetaCexp[ih]) / sigma;
        w              = static_cast<float>(-0.5 * d * d);
        npe            = static_cast<float>(r.nBestHyp[ih]);
      }
      pid.addToHypotheses(edm4eic::CherenkovParticleIDHypothesis{
          .PDG = m_cfg.hypotheses[ih], .npe = npe, .weight = w});
    }
    for (const auto& [th, ph] : r.thetaPhi) {
      pid.addToThetaPhiPhotons(edm4hep::Vector2f{static_cast<float>(th), static_cast<float>(ph)});
    }
    // rawHits: not used (photons are pixelized internally, see DircGR.h)

    // ---- IrtRadiatorInfo (angle in mrad, as IrtInterface) ----
    ri.setAngle(hasPID ? static_cast<float>(r.thetaCPeak) : 0.f);
    ri.setNpe(static_cast<std::uint16_t>(hasPID ? std::min(r.nPhotonsUsed, 65535) : 0));
    ri.setNhits(static_cast<std::uint16_t>(std::min(r.nHitsBox, 65535)));

    if (!ok) {
      continue;
    }
    debug("segment box {} bar {} z_in={:.1f} p={:.3f}: hits box/pix {}/{}, pairs tight/gated "
          "{}/{}, photons used {}, thetaC mean/peak {:.2f}/{:.2f} mrad, sigma {:.2f}, best hyp {}",
          g.box, g.bar, g.zIn, g.ptot, r.nHitsBox, r.nHitsPix, r.nPairsTight, r.nPairsGated,
          r.nPhotonsUsed, r.thetaCMean, r.thetaCPeak, sigma, fmt::join(r.nBestHyp, ","));
  }
}

} // namespace eicrecon
