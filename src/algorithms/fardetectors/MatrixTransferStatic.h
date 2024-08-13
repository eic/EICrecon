// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023 Alex Jentsch, Wouter Deconinck, Sylvester Joosten, David Lawrence, Simon Gardner
//
// This converted from: https://eicweb.phy.anl.gov/EIC/juggler/-/blob/master/JugReco/src/components/FarForwardParticles.cpp

#include <DD4hep/Detector.h>
#include <DDRec/CellIDPositionConverter.h>
#include <algorithms/algorithm.h>
#include <algorithms/geo.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4eic/TrackerHitCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <gsl/pointers>
#include <string>
#include <string_view>

#include "MatrixTransferStaticConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

  using MatrixTransferStaticAlgorithm = algorithms::Algorithm<
    algorithms::Input<
      edm4hep::MCParticleCollection,
      edm4hep::MCParticleCollection,
      edm4eic::TrackerHitCollection
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
                            {"mcParticles", "inputHitCollection"},
                            {"outputParticleCollection"},
                            "Apply matrix method reconstruction to hits."} {}

    void init() final;
    void process(const Input&, const Output&);
    bool initalizeMatrix(const edm4hep::MCParticleCollection&);

  private:
    const dd4hep::Detector* m_detector{algorithms::GeoSvc::instance().detector()};
    const dd4hep::rec::CellIDPositionConverter* m_converter{algorithms::GeoSvc::instance().cellIDPositionConverter()};
    std::once_flag m_initBeamE;
    bool m_initialized{false};
    double m_nomMomentum{0.0};
    double m_local_x_offset{0.0};
    double m_local_y_offset{0.0};
    double m_local_x_slope_offset{0.0};
    double m_local_y_slope_offset{0.0};

    double m_aXinv[2][2] = {{0.0, 0.0},
                           {0.0, 0.0}};
    double m_aYinv[2][2] = {{0.0, 0.0},
                           {0.0, 0.0}};

  };
}
