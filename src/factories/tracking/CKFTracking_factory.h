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
#include "extensions/jana/JOmniAlgorithmFactory.h"
#include "services/geometry/acts/ACTSGeo_service.h"

namespace eicrecon {

class CKFTracking_factory
    : public JOmniAlgorithmFactory<CKFTracking_factory, eicrecon::CKFTracking, CKFTrackingConfig> {
private:
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
  ParameterRef<double> m_maxQOverPVariance{
      this, "MaxQOverPVariance", config().maxQOverPVariance,
      "Drop a CKF branch once its filtered variance of q/p exceeds this"};
  ParameterRef<bool> m_useTime{this, "UseTime", config().useTime,
                               "Include measurement time in the ACTS CKF measurement calibration"};

  Service<ACTSGeo_service> m_ACTSGeoSvc{this};
};

} // namespace eicrecon
