// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 W.J. Llope
//
// Per-photon chromatic-calibration dump of DircGR (DircGRConfig::calibFile):
// one TFile with the TTree "chrcalib", filled by all DircGR instances
// (threads) under a mutex. DircGRCalibSvc hands out one shared instance per
// file name; the tree is written and the file closed when the last DircGR
// using it goes away (i.e. at the end of the run, not at static destruction).
// Input of DircGR_Calibration chrcorr/chrcorrfit.C.

#pragma once

#include <mutex>
#include <span>
#include <string>

class TFile;
class TTree;

namespace eicrecon {

class DircGRCalibDump {
public:
  struct Row {
    int zdir;       // -1 direct (towards the prism), +1 via the mirror
    double dtOverL; // (t_hit - t_exp) / path length [ns/mm]
    double thetaC;  // uncorrected thetaC of the best hypothesis [mrad]
    double lambda;  // photon wavelength [nm] (MC truth; 0 without truth links)
  };

  /// Create (RECREATE) `filename`; throws if it cannot be opened.
  explicit DircGRCalibDump(const std::string& filename);
  ~DircGRCalibDump();
  DircGRCalibDump(const DircGRCalibDump&)            = delete; // owns the open file
  DircGRCalibDump& operator=(const DircGRCalibDump&) = delete;
  DircGRCalibDump(DircGRCalibDump&&)                 = delete;
  DircGRCalibDump& operator=(DircGRCalibDump&&)      = delete;

  /// Rows of one track: detector polar angle [deg], |p| at the bar [GeV],
  /// bar, expected thetaC of the first hypothesis [mrad]. Thread safe.
  void write(double thetaDeg, double p, int bar, double thetaCexp, std::span<const Row> rows);

private:
  std::mutex m_mtx;
  TFile* m_file{nullptr};
  TTree* m_tree{nullptr}; // owned by m_file
  float m_thetaDeg{0}, m_p{0}, m_dtOverL{0}, m_thetaC{0}, m_thetaCexp{0}, m_lambda{0};
  int m_zdir{0}, m_bar{0};
};

} // namespace eicrecon
