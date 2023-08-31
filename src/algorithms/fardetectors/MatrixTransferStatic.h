// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023 Alex Jentsch, Wouter Deconinck, Sylvester Joosten, David Lawrence, Simon Gardner
//
// This converted from: https://eicweb.phy.anl.gov/EIC/juggler/-/blob/master/JugReco/src/components/FarForwardParticles.cpp

#include <spdlog/spdlog.h>

#include <DDRec/CellIDPositionConverter.h>

// Event Model related classes
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <edm4eic/vector_utils.h>

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

    void init(std::shared_ptr<spdlog::logger> &logger);

    std::unique_ptr<edm4eic::ReconstructedParticleCollection> produce(const edm4hep::SimTrackerHitCollection &inputhits);

    // Get position convertor
    std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> getGeoConverter() {return m_cellid_converter;}

    // Set position convertor
    void setGeoConverter(std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> id_conv) {m_cellid_converter=id_conv;}

    // Set position convertor
    void setDetector(dd4hep::Detector* det) {m_detector=det;}

  private:
    /** configuration parameters **/
    eicrecon::MatrixTransferStaticConfig m_cfg;

    /** algorithm logger */
    std::shared_ptr<spdlog::logger>   m_log;
    dd4hep::Detector* m_detector{nullptr};
    std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> m_cellid_converter = nullptr;

  };
}
