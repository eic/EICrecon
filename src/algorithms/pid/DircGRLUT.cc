// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 W.J. Llope

#include "DircGRLUT.h"

#include <RtypesCore.h>
#include <TFile.h>
#include <TParameter.h>
#include <TTree.h>
#include <fmt/format.h>
#include <chrono>
#include <initializer_list>
#include <map>
#include <mutex>
#include <stdexcept>

namespace eicrecon {

namespace {

  int readGridPar(TFile& f, const char* name, const std::string& fname) {
    auto* p = f.Get<TParameter<int>>(name);
    if (p == nullptr) {
      throw std::runtime_error(fmt::format(
          "DircGRLUT: no TParameter<int> {} in {} (LUT without grid info)", name, fname));
    }
    return p->GetVal();
  }

} // namespace

std::shared_ptr<const DircGRLUT> DircGRLUT::get(const std::string& dir) {
  // Called from DircGR::init() only (once per factory instance, i.e. per
  // thread); the event loop uses the returned table without locking.
  static std::mutex mtx;
  static std::map<std::string, std::weak_ptr<const DircGRLUT>> cache;
  std::lock_guard<std::mutex> lock(mtx);
  if (auto sp = cache[dir].lock()) {
    return sp;
  }
  std::shared_ptr<const DircGRLUT> sp(new DircGRLUT(dir));
  cache[dir] = sp;
  return sp;
}

DircGRLUT::DircGRLUT(const std::string& dir) {
  const auto t0 = std::chrono::steady_clock::now();

  // pass 1: read everything (entries are not guaranteed to be cell-sorted)
  struct Row {
    std::uint32_t cell;
    Path path;
  };
  std::vector<Row> rows;
  std::vector<std::uint32_t> count;
  for (int ifile = 0; ifile == 0 || ifile < m_nb; ifile++) {
    const std::string fname = fmt::format("{}/lut_bar{}.root", dir, ifile);
    std::unique_ptr<TFile> f(TFile::Open(fname.c_str(), "READ"));
    if (!f || f->IsZombie()) {
      throw std::runtime_error(fmt::format("DircGRLUT: cannot open {}", fname));
    }
    // grid: taken from lut_bar0, all other files must agree
    const int nx  = readGridPar(*f, "nPixX", fname);
    const int ny  = readGridPar(*f, "nPixY", fname);
    const int nb  = readGridPar(*f, "nBar", fname);
    const int bar = readGridPar(*f, "bar", fname);
    if (ifile == 0) {
      if (nx <= 0 || ny <= 0 || nb <= 0) {
        throw std::runtime_error(
            fmt::format("DircGRLUT: invalid grid {}x{}, {} bars in {}", nx, ny, nb, fname));
      }
      m_nx = nx;
      m_ny = ny;
      m_nb = nb;
      count.assign(static_cast<std::size_t>(m_nx) * m_ny * m_nb, 0);
    } else if (nx != m_nx || ny != m_ny || nb != m_nb) {
      throw std::runtime_error(fmt::format("DircGRLUT: grid {}x{}, {} bars in {} differs from "
                                           "{}x{}, {} bars in lut_bar0.root",
                                           nx, ny, nb, fname, m_nx, m_ny, m_nb));
    }
    // lut_bar{i}.root holds the paths of DIRCHits cellID bar i
    if (bar != ifile) {
      throw std::runtime_error(fmt::format("DircGRLUT: {} says bar {}", fname, bar));
    }
    auto* tree = f->Get<TTree>("dircLUT");
    if (tree == nullptr) {
      throw std::runtime_error(fmt::format("DircGRLUT: no dircLUT tree in {}", fname));
    }
    int ipixx = 0;
    int ipixy = 0;
    double dx = 0;
    double dy = 0;
    double dz = 0;
    double t  = 0;
    tree->SetBranchStatus("*", false);
    for (const char* b : {"ipixx", "ipixy", "OPdirx", "OPdiry", "OPdirz", "OPtime"}) {
      tree->SetBranchStatus(b, true);
    }
    tree->SetBranchAddress("ipixx", &ipixx);
    tree->SetBranchAddress("ipixy", &ipixy);
    tree->SetBranchAddress("OPdirx", &dx);
    tree->SetBranchAddress("OPdiry", &dy);
    tree->SetBranchAddress("OPdirz", &dz);
    tree->SetBranchAddress("OPtime", &t);
    const auto n = tree->GetEntries();
    rows.reserve(rows.size() + n);
    for (Long64_t i = 0; i < n; i++) {
      tree->GetEntry(i);
      if (ipixx < 0 || ipixx >= m_nx || ipixy < 0 || ipixy >= m_ny) {
        throw std::runtime_error(fmt::format("DircGRLUT: pixel ({},{}) outside {}x{} grid in {}",
                                             ipixx, ipixy, m_nx, m_ny, fname));
      }
      const auto c = static_cast<std::uint32_t>(cell(ipixx, ipixy, bar));
      rows.push_back({.cell = c, .path = {.dir = {dx, dy, dz}, .time = t}});
      ++count[c];
    }
  }

  // pass 2: CSR layout, preserving file order within each cell
  const auto ncells = count.size();
  m_begin.assign(ncells + 1, 0);
  for (std::size_t c = 0; c < ncells; c++) {
    m_begin[c + 1] = m_begin[c] + count[c];
  }
  m_paths.resize(rows.size());
  std::vector<std::uint32_t> fill(m_begin.begin(), m_begin.end() - 1);
  for (const auto& r : rows) {
    m_paths[fill[r.cell]++] = r.path;
  }
  m_load_s = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
}

std::span<const DircGRLUT::Path> DircGRLUT::paths(int ix, int iy, int bar) const {
  if (ix < 0 || ix >= m_nx || iy < 0 || iy >= m_ny || bar < 0 || bar >= m_nb) {
    return {};
  }
  const auto c = cell(ix, iy, bar);
  return std::span(m_paths).subspan(m_begin[c], m_begin[c + 1] - m_begin[c]);
}

std::size_t DircGRLUT::bytes() const {
  return m_begin.size() * sizeof(std::uint32_t) + m_paths.size() * sizeof(Path);
}

} // namespace eicrecon
