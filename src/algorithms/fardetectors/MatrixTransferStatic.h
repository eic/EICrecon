// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Alex Jentsch, Wouter Deconinck, Sylvester Joosten, David Lawrence
//
// This converted from: https://eicweb.phy.anl.gov/EIC/juggler/-/blob/master/JugReco/src/components/FarForwardParticles.cpp

#include <algorithm>
#include <cmath>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <DDRec/CellIDPositionConverter.h>
#include <DDRec/Surface.h>
#include <DDRec/SurfaceManager.h>


// Event Model related classes
#include <edm4eic/ReconstructedParticleCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <edm4eic/vector_utils.h>

#include "MatrixTransferStaticConfig.h"

namespace eicrecon {

  class MatrixTransferStatic {
  public:
    MatrixTransferStatic() = default;

    //----- Define constants here ------
    std::string m_readout;
    std::string m_layerField;
    std::string m_sectorField;

    dd4hep::BitFieldCoder *id_dec = nullptr;
    size_t sector_idx{0}, layer_idx{0};

    std::string m_localDetElement;
    std::vector<std::string> u_localDetFields;


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

    // Get a configuration to be changed
    eicrecon::MatrixTransferStaticConfig& getConfig() {return m_cfg;}

    // Sets a configuration (config is properly copyible)
    eicrecon::MatrixTransferStaticConfig& applyConfig(eicrecon::MatrixTransferStaticConfig cfg) { m_cfg = cfg; return m_cfg;}

  private:
    /** configuration parameters **/
    eicrecon::MatrixTransferStaticConfig m_cfg;

    /** algorithm logger */
    std::shared_ptr<spdlog::logger>   m_log;
    dd4hep::Detector* m_detector{nullptr};
    std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> m_cellid_converter = nullptr;

  };
}
