// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <ActsExamples/EventData/Track.hpp>
#include <JANA/JEvent.h>
#include <edm4eic/TrackParametersCollection.h>
#include <edm4eic/TrajectoryCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "algorithms/tracking/CKFTracking.h"
#include "algorithms/tracking/CKFTrackingConfig.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/geometry/acts/ACTSGeo_service.h"

namespace eicrecon {

class CKFTracking_factory : public JOmniFactory<CKFTracking_factory, CKFTrackingConfig> {

private:
  using AlgoT = eicrecon::CKFTracking;
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::TrackSeed> m_seeds_input{this};
  PodioInput<edm4eic::Measurement2D> m_measurements_input{this};
  Output<Acts::ConstVectorMultiTrajectory> m_acts_trajectories_output{this};
  Output<Acts::ConstVectorTrackContainer> m_acts_tracks_output{this};

  ParameterRef<std::vector<double>> m_etaBins{this, "EtaBins", config().etaBins,
                                              "Eta Bins for ACTS CKF tracking reco"};
  ParameterRef<std::vector<double>> m_chi2CutOff{this, "Chi2CutOff", config().chi2CutOff,
                                                 "Chi2 Cut Off for ACTS CKF tracking"};
  ParameterRef<std::vector<std::size_t>> m_numMeasurementsCutOff{
      this, "NumMeasurementsCutOff", config().numMeasurementsCutOff,
      "Number of measurements Cut Off for ACTS CKF tracking"};
  ParameterRef<std::size_t> m_numMeasurementsMin{
      this, "NumMeasurementsMin", config().numMeasurementsMin,
      "Minimum number of measurements for ACTS CKF tracking"};

  Service<ACTSGeo_service> m_ACTSGeoSvc{this};

public:
  void Configure() {
    m_algo = std::make_unique<AlgoT>(this->GetPrefix());
    m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init();
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    // FIXME clear output since it may not have been initialized or reset
    // See https://github.com/eic/EICrecon/issues/1961
    m_acts_trajectories_output().clear();
    m_acts_tracks_output().clear();

    m_algo->process(AlgoT::Input{m_seeds_input(), m_measurements_input()},
                    AlgoT::Output{gsl::not_null{&m_acts_trajectories_output().front()},
                                  gsl::not_null{&m_acts_tracks_output().front()}});
  }
};

} // namespace eicrecon
