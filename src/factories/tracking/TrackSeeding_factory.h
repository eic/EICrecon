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
  using AlgoT    = eicrecon::TrackSeeding;
  using FactoryT = JOmniFactory<TrackSeeding_factory, OrthogonalTrackSeedingConfig>;

  std::unique_ptr<AlgoT> m_algo;

  template <typename T> using PodioInput   = typename FactoryT::template PodioInput<T>;
  template <typename T> using PodioOutput  = typename FactoryT::template PodioOutput<T>;
  template <typename T> using Input        = typename FactoryT::template Input<T>;
  template <typename T> using Output       = typename FactoryT::template Output<T>;
  template <typename T> using ParameterRef = typename FactoryT::template ParameterRef<T>;
  template <typename T> using Service      = typename FactoryT::template Service<T>;

  PodioInput<edm4eic::TrackerHit> m_hits_input{this};
  PodioOutput<edm4eic::TrackParameters> m_parameters_output{this};

  ParameterRef<float> m_rMax{this, "rMax", this->config().rMax,
                             "max measurement radius for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_rMin{this, "rMin", this->config().rMin,
                             "min measurement radius for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_deltaRMinTopSP{this, "deltaRMinTopSP", this->config().deltaRMinTopSP,
                                       "min distance in r between middle and top space point in "
                                       "one seed for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_deltaRMaxTopSP{this, "deltaRMaxTopSP", this->config().deltaRMaxTopSP,
                                       "max distance in r between middle and top space point in "
                                       "one seed for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_deltaRMinBottomSP{this, "deltaRMinBottomSP",
                                          this->config().deltaRMinBottomSP,
                                          "min distance in r between bottom and middle space point "
                                          "in one seed for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_deltaRMaxBottomSP{this, "deltaRMaxBottomSP",
                                          this->config().deltaRMaxBottomSP,
                                          "max distance in r between bottom and middle space point "
                                          "in one seed for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_collisionRegionMin{
      this, "collisionRegionMin", this->config().collisionRegionMin,
      "min location in z for collision region for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_collisionRegionMax{
      this, "collisionRegionMax", this->config().collisionRegionMax,
      "max location in z for collision region for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_zMax{this, "zMax", this->config().zMax,
                             "Max z location for measurements for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_zMin{this, "zMin", this->config().zMin,
                             "Min z location for measurements for Acts::OrthogonalSeedFinder"};
  ParameterRef<unsigned int> m_maxSeedsPerSpM{this, "maxSeedsPerSpM", this->config().maxSeedsPerSpM,
                                              "Maximum number of seeds one space point can be the "
                                              "middle of for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_cotThetaMax{this, "cotThetaMax", this->config().cotThetaMax,
                                    "cot of maximum theta angle for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_sigmaScattering{
      this, "sigmaScattering", this->config().sigmaScattering,
      "number of sigmas of scattering angle to consider for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_radLengthPerSeed{
      this, "radLengthPerSeed", this->config().radLengthPerSeed,
      "Approximate number of radiation lengths one seed traverses for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_minPt{this, "minPt", this->config().minPt,
                              "Minimum pT to search for for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_bFieldInZ{
      this, "bFieldInZ", this->config().bFieldInZ,
      "Value of B Field to use in kiloTesla for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_beamPosX{this, "beamPosX", this->config().beamPosX,
                                 "Beam position in x for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_beamPosY{this, "beamPosY", this->config().beamPosY,
                                 "Beam position in y for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_impactMax{
      this, "impactMax", this->config().impactMax,
      "maximum impact parameter allowed for seeds for Acts::OrthogonalSeedFinder. rMin should be "
      "larger than impactMax."};
  ParameterRef<float> m_rMinMiddle{
      this, "rMinMiddle", this->config().rMinMiddle,
      "min radius for middle space point for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_rMaxMiddle{
      this, "rMaxMiddle", this->config().rMaxMiddle,
      "max radius for middle space point for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_deltaPhiMax{this, "deltaPhiMax", this->config().deltaPhiMax,
                                    "Max phi difference between middle and top/bottom space point"};
  ParameterRef<float> m_locaError{this, "loc_a_Error", this->config().locaError,
                                  "Error on Loc a for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_locbError{this, "loc_b_Error", this->config().locbError,
                                  "Error on Loc b for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_phiError{this, "phi_Error", this->config().phiError,
                                 "Error on phi for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_thetaError{this, "theta_Error", this->config().thetaError,
                                   "Error on theta for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_qOverPError{this, "qOverP_Error", this->config().qOverPError,
                                    "Error on q/p for Acts::OrthogonalSeedFinder"};
  ParameterRef<float> m_timeError{this, "time_Error", this->config().timeError,
                                  "Error on time for Acts::OrthogonalSeedFinder"};

  Service<ACTSGeo_service> m_ACTSGeoSvc{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(GetPrefix());
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    m_algo->process({m_hits_input()}, {m_parameters_output().get()});
  }
};

} // namespace eicrecon
