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
#include <edm4eic/ReconstructedParticle.h>
#include <edm4eic/MutableReconstructedParticle.h>
#include <edm4eic/TrackerHit.h>
#include <edm4hep/SimTrackerHit.h>
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

    dd4hep::DetElement local;
    dd4hep::Detector *detector = nullptr;
    std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> m_cellid_converter = nullptr;

    double aXinv[2][2] = {{0.0, 0.0},
			  {0.0, 0.0}};
    double aYinv[2][2] = {{0.0, 0.0},
			  {0.0, 0.0}};

    void init(std::shared_ptr<spdlog::logger> &logger);

    std::vector<edm4eic::ReconstructedParticle*> produce(const std::vector<const edm4hep::SimTrackerHit *>&);    

    // Get a configuration to be changed
    eicrecon::MatrixTransferStaticConfig& getConfig() {return m_cfg;}
    
    // Sets a configuration (config is properly copyible)
    eicrecon::MatrixTransferStaticConfig& applyConfig(eicrecon::MatrixTransferStaticConfig cfg) { m_cfg = cfg; return m_cfg;}
	
  private:
    /** configuration parameters **/
    eicrecon::MatrixTransferStaticConfig m_cfg;
    
    /** algorithm logger */
    std::shared_ptr<spdlog::logger> m_log;
    
  };
}
