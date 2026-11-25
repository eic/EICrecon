// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 W.J. Llope

#include "DircGRLUT.h"

#include <RtypesCore.h>
#include <TFile.h>
#include <TTree.h>
#include <fmt/format.h>
#include <chrono>
#include <initializer_list>
#include <map>
#include <mutex>
#include <stdexcept>
#include <tuple>
#include <variant>

namespace eicrecon {

std::shared_ptr<const DircGRLUT> DircGRLUT::get(const std::string& dir, int nPixX, int nPixY,
                                                int nBar) {
  static std::mutex mtx;
  static std::map<std::tuple<std::string, int, int, int>, std::weak_ptr<const DircGRLUT>> cache;
  std::lock_guard<std::mutex> lock(mtx);
  auto key = std::make_tuple(dir, nPixX, nPixY, nBar);
  if (auto sp = cache[key].lock()) {
    return sp;
  }
  std::shared_ptr<const DircGRLUT> sp(new DircGRLUT(dir, nPixX, nPixY, nBar));
  cache[key] = sp;
  return sp;
}

DircGRLUT::DircGRLUT(const std::string& dir, int nx, int ny, int nb)
    : m_nx(nx), m_ny(ny), m_nb(nb) {
  const auto t0     = std::chrono::steady_clock::now();
  const auto ncells = static_cast<std::size_t>(nx) * ny * nb;

  // pass 1: read everything (entries are not guaranteed to be cell-sorted)
  struct Row {
    std::uint32_t cell;
    double prob, dx, dy, dz, t;
  };
  std::vector<Row> rows;
  std::vector<std::uint32_t> count(ncells, 0);
  for (int ifile = 0; ifile < nb; ifile++) {
    // lut_bar{i}.root holds the paths of DIRCHits cellID bar i
    const int ibar          = ifile;
    const std::string fname = fmt::format("{}/lut_bar{}.root", dir, ifile);
    std::unique_ptr<TFile> f(TFile::Open(fname.c_str(), "READ"));
    if (!f || f->IsZombie()) {
      throw std::runtime_error(fmt::format("DircGRLUT: cannot open {}", fname));
    }
    auto* tree = f->Get<TTree>("dircLUT");
    if (tree == nullptr) {
      throw std::runtime_error(fmt::format("DircGRLUT: no dircLUT tree in {}", fname));
    }
    int ipixx   = 0;
    int ipixy   = 0;
    double prob = 0;
    double dx   = 0;
    double dy   = 0;
    double dz   = 0;
    double t    = 0;
    tree->SetBranchStatus("*", false);
    for (const char* b : {"ipixx", "ipixy", "OPprob", "OPdirx", "OPdiry", "OPdirz", "OPtime"}) {
      tree->SetBranchStatus(b, true);
    }
    tree->SetBranchAddress("ipixx", &ipixx);
    tree->SetBranchAddress("ipixy", &ipixy);
    tree->SetBranchAddress("OPprob", &prob);
    tree->SetBranchAddress("OPdirx", &dx);
    tree->SetBranchAddress("OPdiry", &dy);
    tree->SetBranchAddress("OPdirz", &dz);
    tree->SetBranchAddress("OPtime", &t);
    const auto n = tree->GetEntries();
    rows.reserve(rows.size() + n);
    for (Long64_t i = 0; i < n; i++) {
      tree->GetEntry(i);
      if (ipixx < 0 || ipixx >= nx || ipixy < 0 || ipixy >= ny) {
        throw std::runtime_error(fmt::format("DircGRLUT: pixel ({},{}) outside {}x{} grid in {}",
                                             ipixx, ipixy, nx, ny, fname));
      }
      const auto c = static_cast<std::uint32_t>(cell(ipixx, ipixy, ibar));
      rows.push_back({c, prob, dx, dy, dz, t});
      ++count[c];
    }
  }

  // pass 2: CSR layout, preserving file order within each cell (as the old
  // code's push_back did)
  m_begin.assign(ncells + 1, 0);
  for (std::size_t c = 0; c < ncells; c++) {
    m_begin[c + 1] = m_begin[c] + count[c];
  }
  const auto N = rows.size();
  m_prob.resize(N);
  m_dir.resize(N);
  m_time.resize(N);
  std::vector<std::uint32_t> fill(m_begin.begin(), m_begin.end() - 1);
  for (const auto& r : rows) {
    const auto k = fill[r.cell]++;
    m_prob[k]    = r.prob;
    m_dir[k]     = edm4hep::Vector3d{r.dx, r.dy, r.dz};
    m_time[k]    = r.t;
  }
  m_load_s = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
}

DircGRLUT::Paths DircGRLUT::paths(int ix, int iy, int bar) const {
  if (ix < 0 || ix >= m_nx || iy < 0 || iy >= m_ny || bar < 0 || bar >= m_nb) {
    return {};
  }
  const auto c = cell(ix, iy, bar);
  const auto b = m_begin[c];
  const auto n = m_begin[c + 1] - b;
  return {.prob = std::span(m_prob).subspan(b, n),
          .dir  = std::span(m_dir).subspan(b, n),
          .time = std::span(m_time).subspan(b, n)};
}

std::size_t DircGRLUT::bytes() const {
  return m_begin.size() * sizeof(std::uint32_t) +
         m_prob.size() * (2 * sizeof(double) + sizeof(edm4hep::Vector3d));
}

} // namespace eicrecon
