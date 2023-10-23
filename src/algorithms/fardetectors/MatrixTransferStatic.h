// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023 Alex Jentsch, Wouter Deconinck, Sylvester Joosten, David Lawrence, Simon Gardner
//
// This converted from: https://eicweb.phy.anl.gov/EIC/juggler/-/blob/master/JugReco/src/components/FarForwardParticles.cpp

#include <DD4hep/Detector.h>
#include <DDRec/CellIDPositionConverter.h>
// Event Model related classes
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <spdlog/logger.h>
#include <memory>

#include "MatrixTransferStaticConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

  class MatrixTransferStatic : public WithPodConfig<MatrixTransferStaticConfig> {
  public:

    //----- Define constants here ------
    double aXinv[2][2] = {{0.0, 0.0},
                          {0.0, 0.0}};
    double aYinv[2][2] = {{0.0, 0.0},
                          {0.0, 0.0}};

    void init(const dd4hep::Detector* det, const dd4hep::rec::CellIDPositionConverter* id_conv, std::shared_ptr<spdlog::logger> &logger);

    std::unique_ptr<edm4eic::ReconstructedParticleCollection> produce(const edm4hep::SimTrackerHitCollection &inputhits);

  private:

    /** algorithm logger */
    std::shared_ptr<spdlog::logger>   m_log;
    const dd4hep::Detector* m_detector{nullptr};
    const dd4hep::rec::CellIDPositionConverter* m_converter{nullptr};

  };
}
