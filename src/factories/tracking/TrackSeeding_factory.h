// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023  - 2025 Joe Osborn, Dmitry Romanov, Wouter Deconinck

#pragma once

#include <JANA/JEvent.h>
#include <edm4eic/TrackParametersCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/tracking/TrackSeedingConfig.h"
#include "algorithms/tracking/TrackSeeding.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/geometry/acts/ACTSGeo_service.h"

namespace eicrecon {

class TrackSeeding_factory : public JOmniFactory<TrackSeeding_factory, TrackSeedingConfig> {

private:
  using AlgoT = eicrecon::TrackSeeding;
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::TrackerHit> m_hits_input{this};
  PodioOutput<edm4eic::TrackSeed> m_seeds_output{this};
  PodioOutput<edm4eic::TrackParameters> m_trackparams_output{this};

  ParameterRef<float> m_rMax{this, "rMax", config().rMax, "max measurement radius for seeding"};
  ParameterRef<float> m_rMin{this, "rMin", config().rMin, "min measurement radius for seeding"};
  ParameterRef<float> m_deltaRMinTop{this, "deltaRMinTopSP", config().deltaRMinTop,
                                     "min distance in r between middle and top space point"};
  ParameterRef<float> m_deltaRMaxTop{this, "deltaRMaxTopSP", config().deltaRMaxTop,
                                     "max distance in r between middle and top space point"};
  ParameterRef<float> m_deltaRMinBottom{this, "deltaRMinBottomSP", config().deltaRMinBottom,
                                        "min distance in r between bottom and middle space point"};
  ParameterRef<float> m_deltaRMaxBottom{this, "deltaRMaxBottomSP", config().deltaRMaxBottom,
                                        "max distance in r between bottom and middle space point"};
  ParameterRef<float> m_collisionRegionMin{this, "collisionRegionMin", config().collisionRegionMin,
                                           "min location in z for collision region"};
  ParameterRef<float> m_collisionRegionMax{this, "collisionRegionMax", config().collisionRegionMax,
                                           "max location in z for collision region"};
  ParameterRef<float> m_zMax{this, "zMax", config().zMax, "max z location for measurements"};
  ParameterRef<float> m_zMin{this, "zMin", config().zMin, "min z location for measurements"};
  ParameterRef<unsigned int> m_maxSeedsPerSpM{this, "maxSeedsPerSpM", config().maxSeedsPerSpM,
                                              "maximum number of seeds one space point can be the "
                                              "middle of"};
  ParameterRef<float> m_cotThetaMax{this, "cotThetaMax", config().cotThetaMax,
                                    "cot of maximum theta angle"};
  ParameterRef<float> m_sigmaScattering{this, "sigmaScattering", config().sigmaScattering,
                                        "number of sigmas of scattering angle to consider"};
  ParameterRef<float> m_radLengthPerSeed{
      this, "radLengthPerSeed", config().radLengthPerSeed,
      "approximate number of radiation lengths one seed traverses"};
  ParameterRef<float> m_minPt{this, "minPt", config().minPt, "minimum pT to search for"};
  ParameterRef<float> m_bFieldInZ{this, "bFieldInZ", config().bFieldInZ,
                                  "value of B Field to use in Tesla"};
  ParameterRef<float> m_beamPosX{this, "beamPosX", config().beamPosX, "beam position in x"};
  ParameterRef<float> m_beamPosY{this, "beamPosY", config().beamPosY, "beam position in y"};
  ParameterRef<float> m_impactMax{this, "impactMax", config().impactMax,
                                  "maximum impact parameter allowed for seeds"};
  ParameterRef<float> m_rMinMiddle{this, "rMinMiddle", config().rMinMiddle,
                                   "min radius for middle space point"};
  ParameterRef<float> m_rMaxMiddle{this, "rMaxMiddle", config().rMaxMiddle,
                                   "max radius for middle space point"};
  ParameterRef<float> m_deltaPhiMax{this, "deltaPhiMax", config().deltaPhiMax,
                                    "max phi difference between middle and top/bottom space point"};
  ParameterRef<float> m_locaError{this, "loc_a_Error", config().locaError,
                                  "error on loc a for track parameter estimation"};
  ParameterRef<float> m_locbError{this, "loc_b_Error", config().locbError,
                                  "error on loc b for track parameter estimation"};
  ParameterRef<float> m_phiError{this, "phi_Error", config().phiError,
                                 "error on phi for track parameter estimation"};
  ParameterRef<float> m_thetaError{this, "theta_Error", config().thetaError,
                                   "error on theta for track parameter estimation"};
  ParameterRef<float> m_qOverPError{this, "qOverP_Error", config().qOverPError,
                                    "error on q/p for track parameter estimation"};
  ParameterRef<float> m_timeError{this, "time_Error", config().timeError,
                                  "error on time for track parameter estimation"};

  std::string m_seedingMethodStr = "auto";
  ParameterRef<std::string> m_seedingMethod{
      this, "seedingMethod", m_seedingMethodStr,
      "Seeding method: 'auto' (Seeding2 for Acts>=45, Orthogonal for Acts<45), "
      "'seeding2' (requires Acts>=45), 'orthogonal' (always available)"};

  Service<ACTSGeo_service> m_ACTSGeoSvc{this};

public:
  void Configure() {
    // Convert seedingMethod string to enum
    const std::string method = m_seedingMethod();
    if (method == "auto") {
      config().seedingMethod = SeedingMethod::Auto;
    } else if (method == "seeding2") {
      config().seedingMethod = SeedingMethod::Seeding2;
    } else if (method == "orthogonal") {
      config().seedingMethod = SeedingMethod::Orthogonal;
    } else {
      throw std::runtime_error("TrackSeeding: Invalid seedingMethod '" + method +
                               "'. "
                               "Valid values: 'auto', 'seeding2', 'orthogonal'");
    }

    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_hits_input()}, {m_seeds_output().get(), m_trackparams_output().get()});
  }
};

} // namespace eicrecon
