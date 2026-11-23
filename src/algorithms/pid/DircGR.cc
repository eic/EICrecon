// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 hpDIRC GR effort

#include "DircGR.h"

#include <DD4hep/Detector.h>
#include <DD4hep/Readout.h>
#include <algorithms/geo.h>
#include <edm4eic/CherenkovParticleIDHypothesis.h>
#include <edm4eic/TrackPoint.h>
#include <fmt/ranges.h>
#include <stdexcept>
#include <vector>

namespace eicrecon {

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
    warning("chrcorrFile is empty: no chromatic correction (not used yet)");
  }
}

void DircGR::process(const Input& input, const Output& output) const {
  const auto [segments, sim_hits, raw_hits, raw_links] = input;
  auto [pids, radinfo]                                  = output;

  // photon cellIDs from the selected input (skeleton: just counted per box)
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
    const auto points = seg.getPoints();
    if (points.empty()) {
      continue; // keep alignment? segments from DircTruthIncidence always have 1 point
    }
    const auto& pt    = points[0];
    const int module  = static_cast<int>(m_id_dec->get(pt.surface, m_module_idx));
    const int bar     = static_cast<int>(m_id_dec->get(pt.surface, m_bar_idx));
    std::size_t nbox  = 0;
    for (auto c : photon_cells) {
      if (static_cast<int>(m_id_dec->get(c, m_module_idx)) == module) {
        ++nbox;
      }
    }

    // ---- placeholder outputs (Phase 3 fills these) ----
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

    debug("segment box {} bar {} z_in={:.1f} p=({:.3f},{:.3f},{:.3f}): {} photon hits in box",
          module, bar, pt.position.z, pt.momentum.x, pt.momentum.y, pt.momentum.z, nbox);
  }
}

} // namespace eicrecon
