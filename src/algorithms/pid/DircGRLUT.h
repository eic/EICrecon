// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 W.J. Llope
//
// hpDIRC GR look-up table: for each (pixel x, pixel y, bar) the list of
// optical-photon paths (unit direction at the bar, time [ns] from the LUT
// photon start point to the MCP). Stored flat (CSR layout) and shared
// read-only between all DircGR instances (JANA creates one factory per
// thread) through a process-wide cache.
//
// Files: lut_bar{0..nBar-1}.root, each with the TTree "dircLUT" (ipixx,
// ipixy, OPdirx/y/z, OPtime per path) and the grid as TParameter<int>:
// nPixX, nPixY, nBar (same in all files) and bar (= the file's index).

#pragma once

#include <edm4hep/Vector3d.h>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <vector>

namespace eicrecon {

class DircGRLUT {
public:
  struct Path {
    edm4hep::Vector3d dir; // unit direction at the bar
    double time;           // [ns]
  };

  /// Load lut_bar{0..nBar-1}.root from `dir` (grid read from the files), or
  /// return the already-loaded table. paths() is indexed by the DIRCHits
  /// cellID bar (file lut_bar{i} holds bar i). Throws on missing or
  /// inconsistent files.
  static std::shared_ptr<const DircGRLUT> get(const std::string& dir);

  /// paths of pixel (ix, iy) of bar `bar`; empty outside the grid
  std::span<const Path> paths(int ix, int iy, int bar) const;
  int nPixX() const { return m_nx; }
  int nPixY() const { return m_ny; }
  int nBar() const { return m_nb; }
  std::size_t nPathsTotal() const { return m_paths.size(); }
  std::size_t bytes() const;
  double loadSeconds() const { return m_load_s; }

private:
  explicit DircGRLUT(const std::string& dir);
  std::size_t cell(int ix, int iy, int bar) const {
    return (static_cast<std::size_t>(bar) * m_ny + iy) * m_nx + ix;
  }

  int m_nx{0};
  int m_ny{0};
  int m_nb{0};
  std::vector<std::uint32_t> m_begin; // size ncell+1
  std::vector<Path> m_paths;
  double m_load_s{0};
};

} // namespace eicrecon
