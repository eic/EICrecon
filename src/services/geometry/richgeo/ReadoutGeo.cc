// Copyright (C) 2023, Christopher Dilks, Luigi Dello Stritto
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include "ReadoutGeo.h"

// constructor
richgeo::ReadoutGeo::ReadoutGeo(std::string detName_, dd4hep::Detector *det_, std::shared_ptr<spdlog::logger> log_)
  : m_detName(detName_), m_det(det_), m_log(log_)
{
  // capitalize m_detName
  std::transform(m_detName.begin(), m_detName.end(), m_detName.begin(), ::toupper);

  // random number generators
  m_random.SetSeed(1); // default seed

  // default (empty) cellID looper
  m_loopCellIDs = [] (std::function<void(uint64_t)> lambda) { return; };

  // default (empty) cellID rng generator
  m_rngCellIDs = [] (std::function<void(uint64_t)> lambda, float p) { return; };

  // common objects
  m_readoutCoder     = m_det->readout(m_detName+"Hits").idSpec().decoder();
  m_detRich          = m_det->detector(m_detName);
  m_systemID         = m_detRich.id();
  m_cellid_converter = std::make_shared<const dd4hep::rec::CellIDPositionConverter>(*m_det);

  // dRICH readout --------------------------------------------------------------------
  if(m_detName=="DRICH") {

    // get constants from geometry
    m_num_sec           = m_det->constant<int>("DRICH_num_sectors");
    m_num_pdus          = m_det->constant<int>("DRICH_num_pdus");
    m_num_sipms_per_pdu = std::pow(m_det->constant<int>("DRICH_pdu_num_sensors"), 2);
    m_num_px            = m_det->constant<int>("DRICH_num_px");
    m_pixel_size        = m_det->constant<double>("DRICH_pixel_size") / dd4hep::mm;

    // define cellID looper
    m_loopCellIDs = [this] (std::function<void(uint64_t)> lambda) {
      m_log->trace("call VisitAllReadoutPixels for systemID = {} = {}", m_systemID, m_detName);

      // loop over sensors (for all sectors)
      for(auto const& [deName, detSensor] : m_detRich.children()) {
        if(deName.find("sensor_de_sec")!=std::string::npos) {

          // decode `sensorID` to module number and sector number
          auto sensorID = detSensor.id();
          auto ipdu     = m_readoutCoder->get(sensorID, "pdu");
          auto isipm    = m_readoutCoder->get(sensorID, "sipm");
          auto isec     = m_readoutCoder->get(sensorID, "sector");
          // m_log->trace("  module: sensorID={:#018X} => ipdu={:<6} isipm={:<6} isec={:<2} name={}", sensorID, ipdu, isipm, isec, deName);

          // loop over xy-segmentation
          for (int x = 0; x < m_num_px; x++) {
            for (int y = 0; y < m_num_px; y++) {

              auto cellID = cellIDEncoding(isec, ipdu, isipm, x, y);

              // then execute the user's lambda function
              lambda(cellID);
            }
          } // end xy-segmentation loop
        }
      } // end sensor loop (for all sectors)
    }; // end definition of m_loopCellIDs

    // define k random cell IDs generator
    m_rngCellIDs = [this] (std::function<void(uint64_t)> lambda, float p) {
      m_log->trace("call RngReadoutPixels for systemID = {} = {}", m_systemID, m_detName);

      int k = p * m_num_sec * m_num_pdus * m_num_sipms_per_pdu * m_num_px * m_num_px;

      for (int i = 0; i < k; i++) {
	int isec = m_random.Uniform(0., m_num_sec);
	int ipdu = m_random.Uniform(0., m_num_pdus);
	int isipm = m_random.Uniform(0., m_num_sipms_per_pdu);
	int x = m_random.Uniform(0., m_num_px);
	int y = m_random.Uniform(0., m_num_px);

	auto cellID = cellIDEncoding(isec, ipdu, isipm, x, y);

	lambda(cellID);
      }
    };

  }

  // pfRICH readout --------------------------------------------------------------------
  else if(m_detName=="PFRICH") {
    m_log->error("TODO: pfRICH readout bindings have not yet been implemented");
  }

  // ------------------------------------------------------------------------------------------------
  else m_log->error("ReadoutGeo is not defined for detector '{}'",m_detName);

}


// pixel gap mask
// FIXME: generalize; this assumes the segmentation is `CartesianGridXY`
bool richgeo::ReadoutGeo::PixelGapMask(uint64_t cellID, dd4hep::Position pos_hit_global) {
  auto pos_pixel_global = m_cellid_converter->position(cellID);
  auto pos_pixel_local  = GetSensorLocalPosition(cellID, pos_pixel_global);
  auto pos_hit_local    = GetSensorLocalPosition(cellID, pos_hit_global);
  return ! (
      std::abs( pos_hit_local.x()/dd4hep::mm - pos_pixel_local.x()/dd4hep::mm ) > m_pixel_size/2 ||
      std::abs( pos_hit_local.y()/dd4hep::mm - pos_pixel_local.y()/dd4hep::mm ) > m_pixel_size/2
      );
}


// transform global position `pos` to sensor `cellID` frame position
// IMPORTANT NOTE: this has only been tested for the dRICH; if you use it, test it carefully...
dd4hep::Position richgeo::ReadoutGeo::GetSensorLocalPosition(uint64_t cellID, dd4hep::Position pos) {

  // get the VolumeManagerContext for this sensitive detector
  auto context = m_cellid_converter->findContext(cellID);

  // transformation vector buffers
  double xyz_l[3], xyz_e[3], xyz_g[2];
  double pv_g[3], pv_l[3];

  // get sensor position w.r.t. its parent
  auto sensor_elem = context->element;
  sensor_elem.placement().position().GetCoordinates(xyz_l);

  // convert sensor position to global position (cf. `CellIDPositionConverter::positionNominal()`)
  const auto& volToElement = context->toElement();
  volToElement.LocalToMaster(xyz_l, xyz_e);
  const auto& elementToGlobal = sensor_elem.nominal().worldTransformation();
  elementToGlobal.LocalToMaster(xyz_e, xyz_g);
  dd4hep::Position pos_sensor;
  pos_sensor.SetCoordinates(xyz_g);

  // get the position vector of `pos` w.r.t. the sensor position `pos_sensor`
  dd4hep::Direction pos_pv = pos - pos_sensor;

  // then transform it to the sensor's local frame
  pos_pv.GetCoordinates(pv_g);
  volToElement.MasterToLocalVect(pv_g, pv_l);
  dd4hep::Position pos_transformed;
  pos_transformed.SetCoordinates(pv_l);

  // trace log
  /*
  if(m_log->level() <= spdlog::level::trace) {
    m_log->trace("pixel hit on cellID={:#018x}",cellID);
    auto print_pos = [&] (std::string name, dd4hep::Position p) {
      m_log->trace("  {:>30} x={:.2f} y={:.2f} z={:.2f} [mm]: ", name, p.x()/dd4hep::mm,  p.y()/dd4hep::mm,  p.z()/dd4hep::mm);
    };
    print_pos("input position",  pos);
    print_pos("sensor position", pos_sensor);
    print_pos("output position", pos_transformed);
    // auto dim = m_cellid_converter->cellDimensions(cellID);
    // for (std::size_t j = 0; j < std::size(dim); ++j)
    //   m_log->trace("   - dimension {:<5} size: {:.2}",  j, dim[j]);
  }
  */

  return pos_transformed;
}


