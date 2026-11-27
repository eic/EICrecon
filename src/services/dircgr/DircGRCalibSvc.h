// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 W.J. Llope
//
// Service owning the shared resources of DircGR (one factory per thread):
//   - the LUTs (one DircGRLUT per LUT directory, kept for the whole run),
//   - the chromatic corrections (one DircGRChrCorr per file/species/momBin).
// Loading takes a lock; the returned tables are const and used without
// locking in the event loop. Registered by the "dircgr" plugin.

#pragma once

#include <algorithms/logger.h>
#include <algorithms/service.h>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <tuple>

#include "DircGRChrCorr.h"
#include "DircGRLUT.h"

namespace eicrecon {

class DircGRCalibSvc : public algorithms::LoggedService<DircGRCalibSvc> {
public:
  void init() {}

  /// LUT from `dir` (lut_bar{i}.root); loaded on first use. Throws on
  /// missing or inconsistent files.
  const DircGRLUT* lut(const std::string& dir) {
    const std::lock_guard<std::mutex> lock(m_mutex);
    auto& p = m_luts[dir];
    if (!p) {
      info("Loading DircGR LUT from {}", dir);
      p = std::make_unique<const DircGRLUT>(dir);
    }
    return p.get();
  }

  /// Chromatic correction set (species, momBin) from `file`; loaded on first use.
  const DircGRChrCorr* chrcorr(const std::string& file, int species, int momBin) {
    const std::lock_guard<std::mutex> lock(m_mutex);
    auto& p = m_chrcorrs[std::make_tuple(file, species, momBin)];
    if (!p) {
      info("Loading DircGR chromatic correction {} (set {}_{})", file, species, momBin);
      p = std::make_unique<const DircGRChrCorr>(file, species, momBin);
    }
    return p.get();
  }

private:
  std::mutex m_mutex;
  std::map<std::string, std::unique_ptr<const DircGRLUT>> m_luts;
  std::map<std::tuple<std::string, int, int>, std::unique_ptr<const DircGRChrCorr>> m_chrcorrs;

  ALGORITHMS_DEFINE_LOGGED_SERVICE(DircGRCalibSvc);
};

} // namespace eicrecon
