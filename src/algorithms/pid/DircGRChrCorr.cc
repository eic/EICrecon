// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 W.J. Llope

#include "DircGRChrCorr.h"

#include <TFile.h>
#include <TGraph.h>
#include <TObject.h>
#include <fmt/format.h>
#include <algorithm>
#include <cmath>
#include <map>
#include <mutex>
#include <numeric>
#include <span>
#include <stdexcept>
#include <tuple>

namespace eicrecon {

std::shared_ptr<const DircGRChrCorr> DircGRChrCorr::get(const std::string& file, int species,
                                                        int momBin) {
  static std::mutex mtx;
  static std::map<std::tuple<std::string, int, int>, std::weak_ptr<const DircGRChrCorr>> cache;
  std::lock_guard<std::mutex> lock(mtx);
  auto key = std::make_tuple(file, species, momBin);
  if (auto sp = cache[key].lock()) {
    return sp;
  }
  std::shared_ptr<const DircGRChrCorr> sp(new DircGRChrCorr(file, species, momBin));
  cache[key] = sp;
  return sp;
}

DircGRChrCorr::DircGRChrCorr(const std::string& file, int species, int momBin) {
  static const std::array<const char*, kNPar> names{"gChrom1par0", "gChrom1par1", "gChrom2par0",
                                                    "gChrom2par1", "gchr1par0",   "gchr1par1",
                                                    "gchr2par0",   "gchr2par1"};
  std::unique_ptr<TFile> f(TFile::Open(file.c_str(), "READ"));
  if (!f || f->IsZombie()) {
    throw std::runtime_error(fmt::format("DircGRChrCorr: cannot open {}", file));
  }
  std::array<std::unique_ptr<TGraph>, kNPar> graphs;
  for (int p = 0; p < kNPar; p++) {
    const auto gname = fmt::format("{}_Theta_{}_{}", names.at(p), species, momBin);
    auto* g          = f->Get<TGraph>(gname.c_str());
    if (g == nullptr || g->GetN() < 2) {
      throw std::runtime_error(
          fmt::format("DircGRChrCorr: graph {} missing or < 2 points in {}", gname, file));
    }
    graphs.at(p).reset(dynamic_cast<TGraph*>(g->Clone()));
    const auto n = static_cast<std::size_t>(g->GetN());
    const std::span<const double> gx(g->GetX(), n);
    const std::span<const double> gy(g->GetY(), n);
    // sort by x (TGraph::Eval assumes/forces sorted x)
    std::vector<std::size_t> idx(n);
    std::iota(idx.begin(), idx.end(), std::size_t{0});
    std::ranges::sort(idx, [gx](std::size_t a, std::size_t b) { return gx[a] < gx[b]; });
    auto& xs = m_x.at(p);
    auto& ys = m_y.at(p);
    for (auto i : idx) {
      xs.push_back(gx[i]);
      ys.push_back(gy[i]);
    }
  }
  // self check against TGraph::Eval, incl. extrapolation beyond the ends
  for (int p = 0; p < kNPar; p++) {
    for (int k = 0; k <= 720; k++) { // 0 - 180 deg in 0.25 deg steps
      const double th = 0.25 * k;
      m_check = std::max(m_check, std::abs(eval(static_cast<Par>(p), th) - graphs.at(p)->Eval(th)));
    }
  }
}

double DircGRChrCorr::eval(Par p, double x) const {
  const auto& X = m_x.at(p);
  const auto& Y = m_y.at(p);
  const auto n  = X.size();
  // segment [lo, lo+1]; outside the range: linear extrapolation from the
  // first/last segment (as TGraph::Eval)
  std::size_t lo = 0;
  if (x <= X.front()) {
    lo = 0;
  } else if (x >= X.back()) {
    lo = n - 2;
  } else {
    lo = static_cast<std::size_t>(std::ranges::upper_bound(X, x) - X.begin()) - 1;
  }
  const double x0 = X[lo];
  const double x1 = X[lo + 1];
  if (x1 == x0) {
    return Y[lo];
  }
  return Y[lo] + (x - x0) * (Y[lo + 1] - Y[lo]) / (x1 - x0);
}

} // namespace eicrecon
