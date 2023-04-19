// Copyright (C) 2023, Christopher Dilks, Luigi Dello Stritto
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include "ReadoutGeo.h"

// constructor
richgeo::ReadoutGeo::ReadoutGeo(std::string detName_, dd4hep::Detector *det_, bool verbose_)
  : m_detName(detName_), m_det(det_), m_log(Logger::Instance(verbose_))
{
  // capitalize m_detName
  std::transform(m_detName.begin(), m_detName.end(), m_detName.begin(), ::toupper);

  // default (empty) cellID looper
  m_loopCellIDs = [] (std::function<void(uint64_t)> lambda) { return; };

  // default (empty) cellID rng generator
  m_rngCellIDs = [] (std::function<void(uint64_t)> lambda, float p) { return; };

  // common objects
  m_readoutCoder = m_det->readout(m_detName+"Hits").idSpec().decoder();
  m_detRich      = m_det->detector(m_detName);
  m_systemID     = m_detRich.id();

  // dRICH readout --------------------------------------------------------------------
  if(m_detName=="DRICH") {

    // get number of sectors
    m_num_sec = m_det->constant<int>("DRICH_num_sectors");
    // get number of sensors
    m_num_mod = m_det->constant<int>("DRICH_num_sensors");
    // get number of pixels along one side of a SiPM
    m_num_px = m_det->constant<int>("DRICH_num_px");

    // define cellID looper
    m_loopCellIDs = [this] (std::function<void(uint64_t)> lambda) {
      m_log.PrintLog("call VisitAllReadoutPixels for systemID = {} = {}", m_systemID, m_detName);

      // loop over sensors (for all sectors)
      for(auto const& [deName, detSensor] : m_detRich.children()) {
        if(deName.find("sensor_de_sec")!=std::string::npos) {

          // decode `imodsec` to module number and sector number
          auto imodsec = detSensor.id();
          auto imod    = m_readoutCoder->get(imodsec, "module");
          auto isec    = m_readoutCoder->get(imodsec, "sector");
          // m_log.PrintLog("  module: imodsec={:#018X} => imod={:<6} isec={:<2} name={}", imodsec, imod, isec, deName);

          // loop over xy-segmentation
          for (int x = 0; x < m_num_px; x++) {
            for (int y = 0; y < m_num_px; y++) {

              // encode cellID
              dd4hep::long64 cellID_dd4hep;
              m_readoutCoder->set(cellID_dd4hep, "system", m_systemID);
              m_readoutCoder->set(cellID_dd4hep, "sector", isec);
              m_readoutCoder->set(cellID_dd4hep, "module", imod);
              m_readoutCoder->set(cellID_dd4hep, "x",      x);
              m_readoutCoder->set(cellID_dd4hep, "y",      y);
              uint64_t cellID(cellID_dd4hep); // DD4hep uses `dd4hep::long64`, but EDM4hep uses `uint64_t`
              // m_log.PrintLog("    x={:<2} y={:<2} => cellID={:#018X}", x, y, cellID);

              // then execute the user's lambda function
              lambda(cellID);

            }
          } // end xy-segmentation loop
        }
      } // end sensor loop (for all sectors)
    }; // end definition of m_loopCellIDs

    // define k random cell IDs generator
    m_rngCellIDs = [this] (std::function<void(uint64_t)> lambda, float p) {
      m_log.PrintLog("call RngReadoutPixels for systemID = {} = {}", m_systemID, m_detName);

      int k = p*m_num_sec*m_num_mod*m_num_px*m_num_px;

      for (int i = 0; i < k; i++) {
	int isec = m_random.Uniform(0., m_num_sec);
	int imod = m_random.Uniform(0., m_num_mod);
	int x = m_random.Uniform(0., m_num_px);
	int y = m_random.Uniform(0., m_num_px);

	// encode cellID
	dd4hep::long64 cellID_dd4hep;
	m_readoutCoder->set(cellID_dd4hep, "system", m_systemID);
	m_readoutCoder->set(cellID_dd4hep, "sector", isec);
	m_readoutCoder->set(cellID_dd4hep, "module", imod);
	m_readoutCoder->set(cellID_dd4hep, "x",      x);
	m_readoutCoder->set(cellID_dd4hep, "y",      y);
	uint64_t cellID(cellID_dd4hep); // DD4hep uses `dd4hep::long64`, but EDM4hep uses `uint64_t`

	// then execute the user's lambda function
	lambda(cellID);
      }
    };
  }

  // pfRICH readout --------------------------------------------------------------------
  else if(m_detName=="PFRICH") {
    m_log.PrintError("TODO: pfRICH readout bindings have not yet been implemented");
  }

  // ------------------------------------------------------------------------------------------------
  else m_log.PrintError("ReadoutGeo is not defined for detector '{}'",m_detName);

}
