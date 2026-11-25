// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 W.J. Llope
//
// hpDIRC GR look-up table: for each (pixel x, pixel y, bar) the list of
// optical-photon paths (probability, unit direction at the bar, time [ns]
// from the LUT photon start point to the MCP). Stored
// flat (CSR layout) and shared read-only between all DircGR instances
// (JANA creates one factory per thread) through a process-wide cache.

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
  struct Paths {
    std::span<const double> prob;
    std::span<const edm4hep::Vector3d> dir;
    std::span<const double> time; // [ns]
    std::size_t size() const { return prob.size(); }
  };

  /// Load lut_bar{0..nBar-1}.root (tree "dircLUT") from `dir`, or return the
  /// already-loaded table. paths() is indexed by the DIRCHits cellID bar
  /// (file lut_bar{i} holds bar i). Throws on missing files/trees.
  static std::shared_ptr<const DircGRLUT> get(const std::string& dir, int nPixX, int nPixY,
                                              int nBar);

  Paths paths(int ix, int iy, int bar) const;
  int nPixX() const { return m_nx; }
  int nPixY() const { return m_ny; }
  int nBar() const { return m_nb; }
  std::size_t nPathsTotal() const { return m_prob.size(); }
  std::size_t bytes() const;
  double loadSeconds() const { return m_load_s; }

private:
  DircGRLUT(const std::string& dir, int nx, int ny, int nb);
  std::size_t cell(int ix, int iy, int bar) const {
    return (static_cast<std::size_t>(bar) * m_ny + iy) * m_nx + ix;
  }

  int m_nx, m_ny, m_nb;
  std::vector<std::uint32_t> m_begin; // size ncell+1
  std::vector<double> m_prob;
  std::vector<edm4hep::Vector3d> m_dir;
  std::vector<double> m_time;
  double m_load_s{0};
};

} // namespace eicrecon
