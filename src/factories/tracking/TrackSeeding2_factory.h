// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025, EICrecon Authors

#pragma once

#include <JANA/JEvent.h>
#include <edm4eic/TrackParametersCollection.h>
#include <edm4eic/TrackSeedCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/tracking/TripletTrackSeedingConfig.h"
#include "algorithms/tracking/TrackSeeding2.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/geometry/acts/ACTSGeo_service.h"

namespace eicrecon {

class TrackSeeding2_factory
    : public JOmniFactory<TrackSeeding2_factory, TripletTrackSeedingConfig> {

private:
  using AlgoT = eicrecon::TrackSeeding2;
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::TrackerHit> m_hits_input{this};
  PodioOutput<edm4eic::TrackSeed> m_seeds_output{this};
  PodioOutput<edm4eic::TrackParameters> m_trackparams_output{this};

  ParameterRef<float> m_rMax{this, "rMax", config().rMax,
                             "max measurement radius for Acts::TripletSeedFinder"};
  ParameterRef<float> m_rMin{this, "rMin", config().rMin,
                             "min measurement radius for Acts::TripletSeedFinder"};
  ParameterRef<float> m_deltaRMin{this, "deltaRMin", config().deltaRMin,
                                  "min distance in r between doublet space points"};
  ParameterRef<float> m_deltaRMax{this, "deltaRMax", config().deltaRMax,
                                  "max distance in r between doublet space points"};
  ParameterRef<float> m_deltaRMinTop{this, "deltaRMinTop", config().deltaRMinTop,
                                     "min distance in r between middle and top space point"};
  ParameterRef<float> m_deltaRMaxTop{this, "deltaRMaxTop", config().deltaRMaxTop,
                                     "max distance in r between middle and top space point"};
  ParameterRef<float> m_deltaRMinBottom{this, "deltaRMinBottom", config().deltaRMinBottom,
                                        "min distance in r between bottom and middle space point"};
  ParameterRef<float> m_deltaRMaxBottom{this, "deltaRMaxBottom", config().deltaRMaxBottom,
                                        "max distance in r between bottom and middle space point"};
  ParameterRef<float> m_collisionRegionMin{this, "collisionRegionMin", config().collisionRegionMin,
                                           "min z of collision region"};
  ParameterRef<float> m_collisionRegionMax{this, "collisionRegionMax", config().collisionRegionMax,
                                           "max z of collision region"};
  ParameterRef<float> m_zMax{this, "zMax", config().zMax, "max z for seeding"};
  ParameterRef<float> m_zMin{this, "zMin", config().zMin, "min z for seeding"};
  ParameterRef<unsigned int> m_maxSeedsPerSpM{this, "maxSeedsPerSpM", config().maxSeedsPerSpM,
                                              "max seeds per middle space point"};
  ParameterRef<float> m_cotThetaMax{this, "cotThetaMax", config().cotThetaMax,
                                    "cot of max theta angle"};
  ParameterRef<float> m_sigmaScattering{this, "sigmaScattering", config().sigmaScattering,
                                        "number of sigmas of scattering angle to consider"};
  ParameterRef<float> m_radLengthPerSeed{this, "radLengthPerSeed", config().radLengthPerSeed,
                                         "approximate radiation lengths per seed"};
  ParameterRef<float> m_minPt{this, "minPt", config().minPt, "minimum pT"};
  ParameterRef<float> m_bFieldInZ{this, "bFieldInZ", config().bFieldInZ,
                                  "magnetic field in z (Acts units: GeV/[e*mm])"};
  ParameterRef<float> m_beamPosX{this, "beamPosX", config().beamPosX, "beam position x"};
  ParameterRef<float> m_beamPosY{this, "beamPosY", config().beamPosY, "beam position y"};
  ParameterRef<float> m_impactMax{this, "impactMax", config().impactMax, "max impact parameter"};
  ParameterRef<float> m_rMinMiddle{this, "rMinMiddle", config().rMinMiddle,
                                   "min r for middle space point"};
  ParameterRef<float> m_rMaxMiddle{this, "rMaxMiddle", config().rMaxMiddle,
                                   "max r for middle space point"};
  ParameterRef<float> m_deltaPhiMax{this, "deltaPhiMax", config().deltaPhiMax,
                                    "max phi difference for doublet"};
  ParameterRef<float> m_locaError{this, "loc_a_Error", config().locaError,
                                  "error on loc a for track params"};
  ParameterRef<float> m_locbError{this, "loc_b_Error", config().locbError,
                                  "error on loc b for track params"};
  ParameterRef<float> m_phiError{this, "phi_Error", config().phiError,
                                 "error on phi for track params"};
  ParameterRef<float> m_thetaError{this, "theta_Error", config().thetaError,
                                   "error on theta for track params"};
  ParameterRef<float> m_qOverPError{this, "qOverP_Error", config().qOverPError,
                                    "error on q/p for track params"};
  ParameterRef<float> m_timeError{this, "time_Error", config().timeError,
                                  "error on time for track params"};

  Service<ACTSGeo_service> m_ACTSGeoSvc{this};

public:
  void Configure() {
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
