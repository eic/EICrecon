// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 W.J. Llope
//
// hpDIRC GR chromatic correction tables (chrcorr.root). For each detector
// polar angle (deg) the file holds TGraphs of the parameters of two pol1's:
//   - "apparent" (used in reconstruction): dthetaC vs dt/L,
//       gChrom{1,2}par{0,1}_Theta_<spe>_<mom>
//   - "ideal" (truth diagnostics): dthetaC [mrad] vs wavelength [nm],
//       gchr{1,2}par{0,1}_Theta_<spe>_<mom>
// 1 = photons going to the prism at creation (zdir -1), 2 = to the mirror.
// The graphs are copied to plain vectors and evaluated with the same linear
// interpolation/extrapolation as TGraph::Eval (checked at load time), so the
// table is const and shared between all DircGR instances (one per thread).

#pragma once

#include <array>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace eicrecon {

class DircGRChrCorr {
public:
  enum Par {
    kChrom1p0, // apparent, zdir -1: intercept
    kChrom1p1, //                    slope (vs dt/L)
    kChrom2p0, // apparent, zdir +1
    kChrom2p1,
    kChr1p0, // ideal, zdir -1 (vs wavelength [nm], result in mrad)
    kChr1p1,
    kChr2p0, // ideal, zdir +1
    kChr2p1,
    kNPar
  };

  /// Load (or return the already-loaded) table for species/momentum index
  /// (0, 0 = pions at 3 GeV). Throws on missing file/graph.
  static std::shared_ptr<const DircGRChrCorr> get(const std::string& file, int species, int momBin);

  /// parameter `p` at detector polar angle thetaDeg (TGraph::Eval semantics)
  double eval(Par p, double x) const;
  /// max |eval - TGraph::Eval| found by the load-time self check
  double selfCheckMaxDiff() const { return m_check; }
  std::size_t nPoints(Par p) const { return m_x.at(p).size(); }

private:
  DircGRChrCorr(const std::string& file, int species, int momBin);
  std::array<std::vector<double>, kNPar> m_x, m_y;
  double m_check{0};
};

} // namespace eicrecon
