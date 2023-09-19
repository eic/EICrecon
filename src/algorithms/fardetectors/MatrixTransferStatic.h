// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023 Alex Jentsch, Wouter Deconinck, Sylvester Joosten, David Lawrence, Simon Gardner
//
// This converted from: https://eicweb.phy.anl.gov/EIC/juggler/-/blob/master/JugReco/src/components/FarForwardParticles.cpp

#include <memory>

#include "MatrixTransferStaticConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace dd4hep::rec { class CellIDPositionConverter; }
namespace dd4hep { class Detector; }
namespace edm4eic { class ReconstructedParticleCollection; }
namespace edm4hep { class SimTrackerHitCollection; }
namespace spdlog { class logger; }

namespace eicrecon {

  class MatrixTransferStatic : public WithPodConfig<MatrixTransferStaticConfig> {
  public:

    //----- Define constants here ------
    double aXinv[2][2] = {{0.0, 0.0},
                          {0.0, 0.0}};
    double aYinv[2][2] = {{0.0, 0.0},
                          {0.0, 0.0}};

    void init(const std::shared_ptr<const dd4hep::rec::CellIDPositionConverter>,const dd4hep::Detector* det,std::shared_ptr<spdlog::logger> &logger);

    std::unique_ptr<edm4eic::ReconstructedParticleCollection> produce(const edm4hep::SimTrackerHitCollection &inputhits);

  private:

    /** algorithm logger */
    std::shared_ptr<spdlog::logger>   m_log;
    const dd4hep::Detector* m_detector{nullptr};
    std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> m_cellid_converter = nullptr;

  };
}
