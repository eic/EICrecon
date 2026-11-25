// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 W.J. Llope

#include "DircGRCalibDump.h"

#include <TFile.h>
#include <TTree.h>
#include <stdexcept>

namespace eicrecon {

DircGRCalibDump::DircGRCalibDump(const std::string& filename)
    : m_file(TFile::Open(filename.c_str(), "RECREATE")) {
  if (m_file == nullptr || m_file->IsZombie()) {
    delete m_file;
    throw std::runtime_error("DircGRCalibDump: cannot create " + filename);
  }
  m_tree = new TTree("chrcalib", "DircGR per-photon chromatic calibration rows");
  m_tree->Branch("thetaDeg", &m_thetaDeg); // track entry-position polar angle [deg]
  m_tree->Branch("p", &m_p);               // |p| at bar entry [GeV]
  m_tree->Branch("bar", &m_bar);
  m_tree->Branch("zdir", &m_zdir);           // -1 direct, +1 via mirror
  m_tree->Branch("dtOverL", &m_dtOverL);     // [ns/mm]
  m_tree->Branch("thetaC", &m_thetaC);       // uncorrected [mrad]
  m_tree->Branch("thetaCexp", &m_thetaCexp); // expected, first hypothesis [mrad]
  m_tree->Branch("lambda", &m_lambda);       // [nm]
}

DircGRCalibDump::~DircGRCalibDump() {
  m_file->cd();
  m_tree->Write();
  m_file->Close();
  delete m_file; // also deletes m_tree
}

void DircGRCalibDump::write(double thetaDeg, double p, int bar, double thetaCexp,
                            std::span<const Row> rows) {
  const std::lock_guard<std::mutex> lock(m_mtx);
  m_thetaDeg  = static_cast<float>(thetaDeg);
  m_p         = static_cast<float>(p);
  m_bar       = bar;
  m_thetaCexp = static_cast<float>(thetaCexp);
  for (const auto& row : rows) {
    m_zdir    = row.zdir;
    m_dtOverL = static_cast<float>(row.dtOverL);
    m_thetaC  = static_cast<float>(row.thetaC);
    m_lambda  = static_cast<float>(row.lambda);
    m_tree->Fill();
  }
}

} // namespace eicrecon
