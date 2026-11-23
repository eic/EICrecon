// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 W.J. Llope

#include "DircGRChrCorr.h"

#include <TFile.h>
#include <TGraph.h>
#include <fmt/format.h>
#include <algorithm>
#include <cmath>
#include <map>
#include <mutex>
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
    const auto gname = fmt::format("{}_Theta_{}_{}", names[p], species, momBin);
    auto* g          = f->Get<TGraph>(gname.c_str());
    if (g == nullptr || g->GetN() < 2) {
      throw std::runtime_error(
          fmt::format("DircGRChrCorr: graph {} missing or < 2 points in {}", gname, file));
    }
    graphs[p].reset(static_cast<TGraph*>(g->Clone()));
    // sort by x (TGraph::Eval assumes/forces sorted x)
    std::vector<std::size_t> idx(g->GetN());
    for (std::size_t i = 0; i < idx.size(); i++) {
      idx[i] = i;
    }
    std::sort(idx.begin(), idx.end(),
              [g](std::size_t a, std::size_t b) { return g->GetX()[a] < g->GetX()[b]; });
    for (auto i : idx) {
      m_x[p].push_back(g->GetX()[i]);
      m_y[p].push_back(g->GetY()[i]);
    }
  }
  // self check against TGraph::Eval, incl. extrapolation beyond the ends
  for (int p = 0; p < kNPar; p++) {
    for (double th = 0; th <= 180.0; th += 0.25) {
      m_check = std::max(m_check, std::abs(eval(static_cast<Par>(p), th) - graphs[p]->Eval(th)));
    }
  }
}

double DircGRChrCorr::eval(Par p, double x) const {
  const auto& X = m_x[p];
  const auto& Y = m_y[p];
  const auto n  = X.size();
  // segment [lo, lo+1]; outside the range: linear extrapolation from the
  // first/last segment (as TGraph::Eval)
  std::size_t lo = 0;
  if (x <= X.front()) {
    lo = 0;
  } else if (x >= X.back()) {
    lo = n - 2;
  } else {
    lo = static_cast<std::size_t>(std::upper_bound(X.begin(), X.end(), x) - X.begin()) - 1;
  }
  const double x0 = X[lo], x1 = X[lo + 1];
  if (x1 == x0) {
    return Y[lo];
  }
  return Y[lo] + (x - x0) * (Y[lo + 1] - Y[lo]) / (x1 - x0);
}

} // namespace eicrecon
