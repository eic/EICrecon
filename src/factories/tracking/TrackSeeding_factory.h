// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023  - 2025 Joe Osborn, Dmitry Romanov, Wouter Deconinck

#pragma once

#include <JANA/JEvent.h>
#include <edm4eic/TrackParametersCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/tracking/OrthogonalTrackSeedingConfig.h"
#include "algorithms/tracking/TrackSeeding.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/geometry/acts/ACTSGeo_service.h"

namespace eicrecon {

class TrackSeeding_factory
    : public JOmniFactory<TrackSeeding_factory, OrthogonalTrackSeedingConfig> {

private:
  using AlgoT = eicrecon::TrackSeeding;
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::TrackerHit> m_hits_input{this};
  PodioOutput<edm4eic::TrackSeed> m_seeds_output{this};
  PodioOutput<edm4eic::TrackParameters> m_trackparams_output{this};

  ParameterRef<float> m_rMax{this, "rMax", config().rMax,
                             "max measurement radius for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_rMin{this, "rMin", config().rMin,
                             "min measurement radius for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_deltaRMinTopSP{this, "deltaRMinTopSP", config().deltaRMinTopSP,
                                       "min distance in r between middle and top space point in "
                                       "one seed for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_deltaRMaxTopSP{this, "deltaRMaxTopSP", config().deltaRMaxTopSP,
                                       "max distance in r between middle and top space point in "
                                       "one seed for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_deltaRMinBottomSP{this, "deltaRMinBottomSP", config().deltaRMinBottomSP,
                                          "min distance in r between bottom and middle space point "
                                          "in one seed for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_deltaRMaxBottomSP{this, "deltaRMaxBottomSP", config().deltaRMaxBottomSP,
                                          "max distance in r between bottom and middle space point "
                                          "in one seed for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_collisionRegionMin{
      this, "collisionRegionMin", config().collisionRegionMin,
      "min location in z for collision region for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_collisionRegionMax{
      this, "collisionRegionMax", config().collisionRegionMax,
      "max location in z for collision region for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_zMax{this, "zMax", config().zMax,
                             "Max z location for measurements for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_zMin{this, "zMin", config().zMin,
                             "Min z location for measurements for Acts::OrthogonalSeedFinder"};
  ParameterRef<unsigned int> m_maxSeedsPerSpM{this, "maxSeedsPerSpM", config().maxSeedsPerSpM,
                                              "Maximum number of seeds one space point can be the "
                                              "middle of for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_cotThetaMax{this, "cotThetaMax", config().cotThetaMax,
                                    "cot of maximum theta angle for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_sigmaScattering{
      this, "sigmaScattering", config().sigmaScattering,
      "number of sigmas of scattering angle to consider for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_radLengthPerSeed{
      this, "radLengthPerSeed", config().radLengthPerSeed,
      "Approximate number of radiation lengths one seed traverses for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_minPt{this, "minPt", config().minPt,
                              "Minimum pT to search for for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_bFieldInZ{
      this, "bFieldInZ", config().bFieldInZ,
      "Value of B Field to use in kiloTesla for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_beamPosX{this, "beamPosX", config().beamPosX,
                                 "Beam position in x for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_beamPosY{this, "beamPosY", config().beamPosY,
                                 "Beam position in y for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_impactMax{
      this, "impactMax", config().impactMax,
      "maximum impact parameter allowed for seeds for Acts::OrthogonalSeedFinder. rMin should be "
      "larger than impactMax."};
  ParameterRef<float> m_rMinMiddle{
      this, "rMinMiddle", config().rMinMiddle,
      "min radius for middle space point for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_rMaxMiddle{
      this, "rMaxMiddle", config().rMaxMiddle,
      "max radius for middle space point for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_deltaPhiMax{this, "deltaPhiMax", config().deltaPhiMax,
                                    "Max phi difference between middle and top/bottom space point"};
  ParameterRef<float> m_locaError{this, "loc_a_Error", config().locaError,
                                  "Error on Loc a for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_locbError{this, "loc_b_Error", config().locbError,
                                  "Error on Loc b for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_phiError{this, "phi_Error", config().phiError,
                                 "Error on phi for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_thetaError{this, "theta_Error", config().thetaError,
                                   "Error on theta for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_qOverPError{this, "qOverP_Error", config().qOverPError,
                                    "Error on q/p for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_timeError{this, "time_Error", config().timeError,
                                  "Error on time for Acts::OrthogonalSeedFinder"};

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
