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

#include "ActsExamples/EventData/Trajectories.hpp"
#include "algorithms/tracking/CKFTracking.h"
#include "algorithms/tracking/CKFTrackingConfig.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/geometry/acts/ACTSGeo_service.h"

namespace eicrecon {

class CKFTracking_factory : public JOmniFactory<CKFTracking_factory, CKFTrackingConfig> {

private:
  using AlgoT = eicrecon::CKFTracking;
  std::unique_ptr<AlgoT> m_algo;

  PodioInput<edm4eic::TrackParameters> m_parameters_input{this};
  PodioInput<edm4eic::Measurement2D> m_measurements_input{this};
  Output<ActsExamples::Trajectories> m_acts_trajectories_output{this};
  Output<ActsExamples::ConstTrackContainer> m_acts_tracks_output{this};

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
    m_algo = std::make_unique<AlgoT>();
    // TODO: convert CKFTracking to inherit from algorithm::Algorithm
    // m_algo->level(static_cast<algorithms::LogLevel>(logger()->level()));
    m_algo->applyConfig(config());
    m_algo->init(m_ACTSGeoSvc().actsGeoProvider(), logger());
  }

  void Process(int32_t /* run_number */, uint64_t /* event_number */) {
    // FIXME clear output since it may not have been initialized or reset
    // See https://github.com/eic/EICrecon/issues/1961
    m_acts_trajectories_output().clear();
    m_acts_tracks_output().clear();

    std::tie(m_acts_trajectories_output(), m_acts_tracks_output()) =
        m_algo->process(*m_parameters_input(), *m_measurements_input());
  }
};

} // namespace eicrecon
