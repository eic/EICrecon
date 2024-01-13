// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023 Alex Jentsch, Wouter Deconinck, Sylvester Joosten, David Lawrence, Simon Gardner
//
// This converted from: https://eicweb.phy.anl.gov/EIC/juggler/-/blob/master/JugReco/src/components/FarForwardParticles.cpp

#include <DD4hep/Detector.h>
#include <DDRec/CellIDPositionConverter.h>
#include <algorithms/algorithm.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <spdlog/logger.h>
#include <memory>

#include "MatrixTransferStaticConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

  using MatrixTransferStaticAlgorithm = algorithms::Algorithm<
    algorithms::Input<
      edm4hep::SimTrackerHitCollection
    >,
    algorithms::Output<
      edm4eic::ReconstructedParticleCollection
    >
  >;

  class MatrixTransferStatic
  : public MatrixTransferStaticAlgorithm,
    public WithPodConfig<MatrixTransferStaticConfig> {

  public:
    MatrixTransferStatic(std::string_view name)
      : MatrixTransferStaticAlgorithm{name,
                            {"inputHitCollection"},
                            {"outputParticleCollection"},
                            "Apply matrix method reconstruction to hits."} {}

    void init(const dd4hep::Detector* detector, const dd4hep::rec::CellIDPositionConverter* id_conv, std::shared_ptr<spdlog::logger>& logger);
    void process(const Input&, const Output&) const final;

  private:

    //----- Define constants here ------
    double aXinv[2][2] = {{0.0, 0.0},
                          {0.0, 0.0}};
    double aYinv[2][2] = {{0.0, 0.0},
                          {0.0, 0.0}};

  private:

    /** algorithm logger */
    std::shared_ptr<spdlog::logger>   m_log;
    const dd4hep::Detector* m_detector{nullptr};
    const dd4hep::rec::CellIDPositionConverter* m_converter{nullptr};

  };
}
