// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 hpDIRC GR effort

#pragma once

#include <edm4eic/CherenkovParticleIDCollection.h>
#include <edm4eic/IrtRadiatorInfoCollection.h>
#include <edm4eic/TrackSegmentCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <memory>
#include <string>
#include <vector>

#include "algorithms/pid/DircGR.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/algorithms_init/AlgorithmsInit_service.h"
#include "services/geometry/dd4hep/DD4hep_service.h"

namespace eicrecon {

class DircGR_factory : public JOmniFactory<DircGR_factory, DircGRConfig> {
public:
  using AlgoT = eicrecon::DircGR;

private:
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::TrackSegment> m_segments_input{this};
  PodioInput<edm4hep::SimTrackerHit> m_photons_input{this};

  PodioOutput<edm4eic::CherenkovParticleID> m_pid_output{this};
  PodioOutput<edm4eic::IrtRadiatorInfo> m_radinfo_output{this};

  ParameterRef<std::string> m_lutPath{this, "lutPath", config().lutPath,
                                      "directory with lut_bar{0..9}.root"};
  ParameterRef<std::string> m_chrcorrFile{this, "chrcorrFile", config().chrcorrFile,
                                          "chromatic correction ROOT file"};
  ParameterRef<int> m_chrcorrSpecies{this, "chrcorrSpecies", config().chrcorrSpecies,
                                     "chrcorr graph set: species index"};
  ParameterRef<int> m_chrcorrMomBin{this, "chrcorrMomBin", config().chrcorrMomBin,
                                    "chrcorr graph set: momentum index"};
  ParameterRef<double> m_refractiveIndex{this, "refractiveIndex", config().refractiveIndex,
                                         "bar refractive index for expected thetaC"};
  ParameterRef<double> m_tightGateHalfWidth0{this, "tightGateHalfWidth0",
                                             config().tightGateHalfWidth0,
                                             "tight time gate half width at tbegOP [ns]"};
  ParameterRef<double> m_tightGateHalfWidth10{this, "tightGateHalfWidth10",
                                              config().tightGateHalfWidth10,
                                              "tight time gate half width at tbegOP+10ns [ns]"};
  ParameterRef<int> m_peakHalfWindow{this, "peakHalfWindow", config().peakHalfWindow,
                                     "thetaC peak estimator half window [1 mrad bins]"};
  ParameterRef<double> m_spr{this, "spr", config().spr, "single-photon resolution [mrad]"};
  ParameterRef<double> m_sigmaExt{this, "sigmaExt", config().sigmaExt,
                                  "track-level thetaC resolution term [mrad]"};
  ParameterRef<int> m_minPhotons{this, "minPhotons", config().minPhotons,
                                 "min photons used for a PID"};
  ParameterRef<std::vector<int>> m_hypotheses{this, "hypotheses", config().hypotheses,
                                              "PDG hypotheses"};
  ParameterRef<double> m_pixelSize{this, "pixelSize", config().pixelSize,
                                   "internal pixel pitch [mm], must match the LUT"};

  Service<AlgorithmsInit_service> m_algorithmsInit{this};
  Service<DD4hep_service> m_geoSvc{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_segments_input(), m_photons_input()},
                    {m_pid_output().get(), m_radinfo_output().get()});
  }
};

} // namespace eicrecon
