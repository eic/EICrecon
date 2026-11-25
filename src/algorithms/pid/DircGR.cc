// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 W.J. Llope

#include "DircGR.h"

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include <Evaluator/DD4hepUnits.h>
#include <TFile.h>
#include <TTree.h>
#include <algorithms/geo.h>
#include <edm4eic/CherenkovParticleIDHypothesis.h>
#include <edm4eic/TrackPoint.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/SimTrackerHit.h>
#include <edm4hep/Vector2f.h>
#include <edm4hep/Vector3d.h>
#include <edm4hep/Vector3f.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <podio/ObjectID.h>
#include <podio/RelationRange.h>
#include <podio/detail/LinkCollectionIterator.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <gsl/pointers>
#include <map>
#include <mutex>
#include <numbers>
#include <span>
#include <stdexcept>
#include <tuple>
#include <vector>

#include "services/particle/ParticleSvc.h"

namespace eicrecon {

// Calibration dump: one TFile/TTree per file name, shared by all DircGR
// instances (threads); written and closed when the last instance goes away.
struct DircGR::CalibDump {
  std::mutex mtx;
  std::unique_ptr<TFile> file;
  TTree* tree{nullptr};
  float thetaDeg{0}, p{0}, dtOverL{0}, thetaC{0}, thetaCexp{0}, lambda{0};
  int zdir{0}, bar{0};
  explicit CalibDump(const std::string& name) : file(TFile::Open(name.c_str(), "RECREATE")) {
    if (!file || file->IsZombie()) {
      throw std::runtime_error("DircGR: cannot create calibFile " + name);
    }
    tree = new TTree("chrcalib", "DircGR per-photon chromatic calibration rows");
    tree->Branch("thetaDeg", &thetaDeg); // track entry-position polar angle [deg]
    tree->Branch("p", &p);               // |p| at bar entry [GeV]
    tree->Branch("bar", &bar);
    tree->Branch("zdir", &zdir);           // -1 direct, +1 via mirror
    tree->Branch("dtOverL", &dtOverL);     // [ns/mm]
    tree->Branch("thetaC", &thetaC);       // uncorrected [mrad]
    tree->Branch("thetaCexp", &thetaCexp); // expected, first hypothesis [mrad]
    tree->Branch("lambda", &lambda);       // [nm]
  }
  CalibDump(const CalibDump&)            = delete; // owns the open file
  CalibDump& operator=(const CalibDump&) = delete;
  CalibDump(CalibDump&&)                 = delete;
  CalibDump& operator=(CalibDump&&)      = delete;
  ~CalibDump() {
    file->cd();
    tree->Write();
    file->Close();
  }
  static std::shared_ptr<CalibDump> get(const std::string& name) {
    static std::mutex m;
    static std::map<std::string, std::weak_ptr<CalibDump>> open;
    std::lock_guard<std::mutex> lock(m);
    auto sp = open[name].lock();
    if (!sp) {
      sp         = std::make_shared<CalibDump>(name);
      open[name] = sp;
    }
    return sp;
  }
};

void DircGR::writeCalib(const DircGRTrackGates& g, const DircGRTrackResult& r) const {
  std::lock_guard<std::mutex> lock(m_calib->mtx);
  auto& d     = *m_calib;
  d.thetaDeg  = static_cast<float>(g.detTheta / dd4hep::deg);
  d.p         = static_cast<float>(g.ptot);
  d.bar       = g.bar;
  d.thetaCexp = static_cast<float>(1000. * g.thetaCexp[0]);
  for (const auto& row : r.calib) {
    d.zdir    = row.zdir;
    d.dtOverL = static_cast<float>(row.dtOverL);
    d.thetaC  = static_cast<float>(row.thetaC);
    d.lambda  = static_cast<float>(row.lambda);
    d.tree->Fill();
  }
}

void DircGR::init() {
  m_id_dec = algorithms::GeoSvc::instance().detector()->readout(m_cfg.readout).idSpec().decoder();
  m_module_idx = m_id_dec->index("module");
  m_bar_idx    = m_id_dec->index("bar");
  m_x_idx      = m_id_dec->index("x");
  m_y_idx      = m_id_dec->index("y");
  // values that end up in denominators / sqrt / acos
  if (m_cfg.timeResolution <= 0 || m_cfg.vBar <= 0 || m_cfg.nBox <= 0 ||
      m_cfg.refractiveIndex <= 1.0 || m_cfg.nPixX <= 0 || m_cfg.nPixY <= 0 || m_cfg.nBar <= 0) {
    throw std::runtime_error("DircGR: invalid config (need timeResolution, vBar, nBox, nPixX, "
                             "nPixY, nBar > 0 and refractiveIndex > 1)");
  }

  // hypothesis masses
  const auto& particleSvc = algorithms::ParticleSvc::instance();
  m_hyp_mass.clear();
  for (int pdg : m_cfg.hypotheses) {
    const double m = particleSvc.particle(pdg).mass;
    m_hyp_mass.push_back(m);
    const double pthr = m / std::sqrt(m_cfg.refractiveIndex * m_cfg.refractiveIndex - 1.0);
    debug("hypothesis PDG {}: mass {:.6f} GeV, Cherenkov threshold {:.3f} GeV (n={})", pdg, m, pthr,
          m_cfg.refractiveIndex);
  }

  debug("hypotheses={} LUT pixel grid {}x{} (MCP cellID grid), bars {}, raw-hit TDC LSB {} ns",
        fmt::join(m_cfg.hypotheses, ","), m_cfg.nPixX, m_cfg.nPixY, m_cfg.nBar,
        m_cfg.timeResolution / edm4eic::unit::ns);
  debug("likelihood: spr={} mrad sigmaExt={} mrad minPhotons={}", m_cfg.spr, m_cfg.sigmaExt,
        m_cfg.minPhotons);

  const auto lut0 = fmt::format("{}/lut_bar0.root", m_cfg.lutPath);
  if (m_cfg.lutPath.empty() || !std::filesystem::exists(lut0)) {
    warning("no LUT ({} not found): DircGR will produce no PID. Set "
            "-PDIRC:DircGR:lutPath=<dir with lut_bar*.root>",
            lut0);
  } else {
    m_lut = DircGRLUT::get(m_cfg.lutPath, m_cfg.nPixX, m_cfg.nPixY, m_cfg.nBar);
    debug("LUT {}: {} paths, {:.1f} MB, loaded in {:.1f} s (shared, use_count={})", m_cfg.lutPath,
          m_lut->nPathsTotal(), m_lut->bytes() / 1048576., m_lut->loadSeconds(), m_lut.use_count());
  }
  if (m_cfg.chrcorrFile.empty() || !std::filesystem::exists(m_cfg.chrcorrFile)) {
    warning("no chromatic correction ({} not found). Set -PDIRC:DircGR:chrcorrFile=<file>",
            m_cfg.chrcorrFile);
  } else {
    m_chr = DircGRChrCorr::get(m_cfg.chrcorrFile, m_cfg.chrcorrSpecies, m_cfg.chrcorrMomBin);
    debug("chrcorr {} (set {}_{}): apparent {}/{} pts, ideal {}/{} pts; self check vs "
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
  if (!m_cfg.calibFile.empty()) {
    if (m_chr != nullptr) {
      warning("calibFile {} requested but chrcorr is loaded: no rows will be written "
              "(run with -PDIRC:DircGR:chrcorrFile= to calibrate)",
              m_cfg.calibFile);
    }
    m_calib = CalibDump::get(m_cfg.calibFile);
    debug("calibration dump: {}", m_cfg.calibFile);
  }
}

bool DircGR::computeGates(const edm4eic::TrackPoint& pt, DircGRTrackGates& g) const {
  g.box = static_cast<int>(m_id_dec->get(pt.surface, m_module_idx));
  g.bar = static_cast<int>(m_id_dec->get(pt.surface, m_bar_idx));
  if (g.box < 0 || g.box >= m_cfg.nBox || g.bar < 0 || g.bar >= m_cfg.nBar) {
    return false;
  }
  const double px = pt.momentum.x;
  const double py = pt.momentum.y;
  const double pz = pt.momentum.z;
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
      g.thetaCexp[i] = std::acos(std::clamp(1.0 / beta / n, -1.0, 1.0));
      g.thetaCexpMax = std::max(g.thetaCexpMax, g.thetaCexp[i]);
    }
  }

  // detector angles from the entry position, direction in the box frame
  const double x    = pt.position.x;
  const double y    = pt.position.y;
  const double z    = pt.position.z;
  const double dist = std::sqrt(x * x + y * y + z * z);
  if (dist <= 0) {
    return false;
  }
  g.detPhi   = std::atan2(y, x);
  g.detTheta = std::acos(std::clamp(z / dist, -1.0, 1.0));
  // eta diverges along the beam axis (theta 0 or pi); the DIRC never sees it
  const double tanHalf = std::tan(g.detTheta / 2.0);
  g.eta                = (tanHalf > 0) ? -std::log(tanHalf) : 0.0;
  const double rot     = -g.box * (2.0 * std::numbers::pi / m_cfg.nBox);
  const double c       = std::cos(rot);
  const double s       = std::sin(rot);
  g.dirRot             = {(c * px - s * py) / g.ptot, (s * px + c * py) / g.ptot, pz / g.ptot};
  g.zIn                = z;
  g.tIn                = pt.time;

  // earliest arrival times
  const double dZ1 = z - m_cfg.zReadout;
  const double dZ2 = (m_cfg.zBarMax - z) + (m_cfg.zBarMax - m_cfg.zReadout);
  g.tbeg1          = dZ1 / m_cfg.vBar + g.tIn;
  g.tbeg2          = dZ2 / m_cfg.vBar + g.tIn;
  double polarOP   = (g.detTheta < std::numbers::pi / 2.) ? g.detTheta - g.thetaCexpMax
                                                          : g.detTheta + g.thetaCexpMax;
  polarOP          = std::clamp(polarOP, 0.0, std::numbers::pi);
  // steepest photon: |cos| bounded away from 0 (a photon at exactly 90 deg
  // never leaves the bar)
  const double cosOP = std::max(std::abs(std::cos(polarOP)), 1e-6);
  g.tbeg1OP          = dZ1 / cosOP / m_cfg.vBar + g.tIn;
  g.tbeg2OP          = dZ2 / cosOP / m_cfg.vBar + g.tIn;
  const double thDeg = g.detTheta / dd4hep::deg; // chrcorr tables are in degrees
  if (g.detTheta > m_cfg.dir1MinTheta && g.detTheta < m_cfg.dir2MaxTheta) {
    g.tbeg1OP -= m_cfg.tbegShift90;
    g.tbeg2OP -= m_cfg.tbegShift90;
  }
  g.use1 = g.detTheta > m_cfg.dir1MinTheta;
  g.use2 = g.detTheta < m_cfg.dir2MaxTheta;

  // envelope of cos(polar angle) of the photon paths: a beta = 1 Cherenkov
  // cone around the track direction spans polar angles thetaTrk -+ thetaC1
  const double thetaTrk = std::acos(std::clamp(g.dirRot[2], -1.0, 1.0));
  const double thetaC1  = std::acos(1.0 / n);
  g.cosAngUpper         = std::cos(std::max(thetaTrk - thetaC1, 0.0)) + m_cfg.cosAngMargin;
  g.cosAngLower = std::cos(std::min(thetaTrk + thetaC1, std::numbers::pi)) - m_cfg.cosAngMargin;

  // chromatic correction parameters at this polar angle
  g.hasChr = (m_chr != nullptr);
  if (g.hasChr) {
    for (int p = 0; p < DircGRChrCorr::kNPar; p++) {
      g.chr.at(p) = m_chr->eval(static_cast<DircGRChrCorr::Par>(p), thDeg);
    }
  }
  return true;
}

DircGR::TimeGate DircGR::timeGateTight(double hitt, double texp1, double texp2,
                                       const DircGRTrackGates& g) const {
  // half width linear in t_exp: hw0 at tbegOP, hw10 at tbegOP + 10 ns
  const double slope =
      (m_cfg.tightGateHalfWidth10 - m_cfg.tightGateHalfWidth0) / (10.0 * edm4eic::unit::ns);
  const double cut1 = m_cfg.tightGateHalfWidth0 + slope * (texp1 - g.tbeg1OP);
  const double cut2 = m_cfg.tightGateHalfWidth0 + slope * (texp2 - g.tbeg2OP);
  const double dt1  = hitt - texp1;
  const double dt2  = hitt - texp2;
  auto gate         = TimeGate::NoMatch;
  if (dt1 >= -cut1 && dt1 < cut1 && g.use1) {
    gate = TimeGate::Direct;
  }
  if (dt2 >= -cut2 && dt2 < cut2 && g.use2) {
    gate = TimeGate::ViaMirror; // direction 2 wins if both match
  }
  if (gate == TimeGate::Direct && hitt < g.tbeg1OP) {
    return TimeGate::DirectTooEarly;
  }
  if (gate == TimeGate::ViaMirror && hitt < g.tbeg2OP) {
    return TimeGate::MirrorTooEarly;
  }
  return gate;
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
  std::vector<double> diffbest(nh);
  std::vector<double> thbest(nh);
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
    const int ix = h.ix;
    const int iy = h.iy;
    if (m_lut == nullptr) {
      continue;
    }
    const auto P = m_lut->paths(ix, iy, g.bar);
    bool used    = false;
    for (std::size_t ip = 0; ip < P.size(); ip++) {
      const double dx    = P.dir[ip].x;
      const double dy    = P.dir[ip].y;
      const double dz    = P.dir[ip].z;
      const double dnorm = std::sqrt(dx * dx + dy * dy + dz * dz);
      if (dnorm <= 0) {
        continue;
      }
      const double OPtheta = std::acos(std::clamp(dz / dnorm, -1.0, 1.0));
      const double acos_th = std::abs(std::cos(OPtheta));
      if (acos_th < 1e-6) { // path perpendicular to the bar axis: never leaves the bar
        continue;
      }
      const double texp1 = P.time[ip] + dZbar1 / acos_th / m_cfg.vBar + g.tIn;
      const double texp2 = P.time[ip] + dZbar2 / acos_th / m_cfg.vBar + g.tIn;

      // ---- time gate (also resolves the z ambiguity), polar-angle gate ----
      const auto gate = timeGateTight(h.t, texp1, texp2, g);
      if (gate != TimeGate::Direct && gate != TimeGate::ViaMirror) {
        continue;
      }
      // photon direction along the bar: -1 towards the prism, +1 via the mirror
      const int zdir = (gate == TimeGate::Direct) ? -1 : +1;
      ++r.nPairsTight;
      if (!cosAngOK(std::cos(OPtheta), zdir, g)) {
        continue;
      }
      ++r.nPairsGated;

      // ---- chromatic correction ("apparent": pol1 in dt / path length) ----
      double chrcorr       = 0;
      const double dt      = h.t - ((zdir == -1) ? texp1 : texp2);
      const double Lpath   = P.time[ip] * m_cfg.vBar + ((zdir == -1) ? dZbar1 : dZbar2) / acos_th;
      const double dtOverL = (Lpath > 0) ? dt / Lpath : 0.0; // [ns/mm]
      if (g.hasChr) {
        chrcorr = (zdir == -1)
                      ? g.chr[DircGRChrCorr::kChrom1p0] + g.chr[DircGRChrCorr::kChrom1p1] * dtOverL
                      : g.chr[DircGRChrCorr::kChrom2p0] + g.chr[DircGRChrCorr::kChrom2p1] * dtOverL;
      }

      // ---- 8-fold direction ambiguity: flip x (bit 0), y (bit 1), z (bit
      // 2); the time gate fixes z: zdir -1 -> no z flip, +1 -> z flip ----
      std::ranges::fill(diffbest, 99999.);
      std::ranges::fill(thbest, 0.);
      std::ranges::fill(ambbest, -1);
      const int a0 = (zdir == -1) ? 0 : 4;
      for (int iamb = a0; iamb < a0 + 4; iamb++) {
        const double sx = ((iamb & 1) != 0) ? -1 : 1;
        const double sy = ((iamb & 2) != 0) ? -1 : 1;
        const double sz = ((iamb & 4) != 0) ? -1 : 1;
        const double th =
            std::acos(std::clamp(sx * dx * u[0] + sy * dy * u[1] + sz * dz * u[2], -1.0, 1.0)) -
            chrcorr;
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
        const double vx = ((a & 1) != 0) ? -dx : dx;
        const double vy = ((a & 2) != 0) ? -dy : dy;
        const double vz = ((a & 4) != 0) ? -dz : dz;
        const double phi =
            std::atan2(vx * e2[0] + vy * e2[1] + vz * e2[2], vx * e1[0] + vy * e1[1] + vz * e1[2]);
        r.thetaPhi.emplace_back(thbest[ibest], phi);
        if (m_calib && !g.hasChr) {
          r.calib.push_back({zdir, dtOverL, 1000. * thbest[ibest], h.lambda});
        }
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
  const int kmax = static_cast<int>(std::ranges::max_element(hist) - hist.begin()) + 1;
  const int klo  = std::max(1, kmax - m_cfg.peakHalfWindow);
  const int khi  = std::min(nbins, kmax + m_cfg.peakHalfWindow);
  double nx      = 0;
  double mx      = 0;
  const double w = hmax / nbins;
  for (int kb = klo; kb <= khi; kb++) {
    nx += hist[kb - 1];
    mx += hist[kb - 1] * (kb - 0.5) * w;
  }
  r.thetaCPeak = (nx > 0) ? mx / nx : 0.;
}

std::vector<DircGR::PixelHit>
DircGR::pixelHits(const edm4eic::RawTrackerHitCollection& hits,
                  const edm4eic::MCRecoTrackerHitLinkCollection& links) const {
  // photon wavelength per raw hit (calibration dump only): the earliest
  // linked photon is the one that set the hit time; lambda[nm] = hc / E
  std::vector<double> lambda(hits.size(), 0.);
  std::vector<double> tbest(hits.size(), 1e30);
  if (m_calib) {
    for (const auto& link : links) {
      const auto raw = link.getFrom();
      const auto sim = link.getTo();
      if (raw.getObjectID().collectionID != hits.getID()) {
        continue;
      }
      const auto k = raw.getObjectID().index;
      if (k < 0 || static_cast<std::size_t>(k) >= hits.size() || sim.getTime() >= tbest[k]) {
        continue;
      }
      tbest[k]        = sim.getTime();
      const auto& mom = sim.getMomentum();
      const double E  = std::sqrt(mom.x * mom.x + mom.y * mom.y + mom.z * mom.z); // [GeV]
      lambda[k]       = (E > 0) ? 1.239841984e-6 / E : 0.;
    }
  }

  const int cy0 = (m_cfg.nPixX - 1) / 2;
  const int cx0 = (m_cfg.nPixY - 1) / 2;
  std::vector<PixelHit> out;
  out.reserve(hits.size());
  for (std::size_t k = 0; k < hits.size(); k++) {
    const auto& h    = hits.at(k);
    const auto id    = h.getCellID();
    const int box    = static_cast<int>(m_id_dec->get(id, m_module_idx));
    const auto cellX = m_id_dec->get(id, m_x_idx);
    const auto cellY = m_id_dec->get(id, m_y_idx);
    int ix           = cy0 - static_cast<int>(cellY);
    int iy           = static_cast<int>(cellX) + cx0;
    if (ix < 0 || ix >= m_cfg.nPixX || iy < 0 || iy >= m_cfg.nPixY) {
      ix = iy = -1;
    }
    const double t = (h.getTimeStamp() + 0.5) * m_cfg.timeResolution; // TDC bin centre [ns]
    out.push_back({box, ix, iy, t, lambda[k]});
  }
  return out;
}

void DircGR::process(const Input& input, const Output& output) const {
  const auto [segments, raw_hits, raw_links] = input;
  auto [pids, radinfo]                       = output;

  const auto photons = pixelHits(*raw_hits, *raw_links);

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
      if (m_calib && !r.calib.empty()) {
        writeCalib(g, r);
      }
    }
    const bool hasPID = ok && m_lut && r.nPhotonsUsed >= m_cfg.minPhotons;

    // ---- CherenkovParticleID ----
    const double sigma =
        hasPID ? std::sqrt(m_cfg.spr * m_cfg.spr / r.nPhotonsUsed + m_cfg.sigmaExt * m_cfg.sigmaExt)
               : 0.; // [mrad]
    pid.setNpe(hasPID ? static_cast<float>(r.nPhotonsUsed) : 0.F);
    for (std::size_t ih = 0; ih < m_cfg.hypotheses.size(); ih++) {
      float w   = 0;
      float npe = 0;
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
    ri.setAngle(hasPID ? static_cast<float>(r.thetaCPeak) : 0.F);
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
