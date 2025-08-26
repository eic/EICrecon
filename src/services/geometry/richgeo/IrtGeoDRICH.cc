// Copyright (C) 2022, 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "IrtGeoDRICH.h"

#include <DD4hep/DetElement.h>
#include <DD4hep/Fields.h>
#include <DD4hep/Objects.h>
#include <DDRec/DetectorData.h>
#include <Evaluator/DD4hepUnits.h>
#include <IRT/CherenkovDetector.h>
#include <IRT/CherenkovDetectorCollection.h>
#include <IRT/CherenkovRadiator.h>
#include <IRT/G4Object.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <Parsers/Primitives.h>
#include <TRef.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <cstdint>
#include <map>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

void richgeo::IrtGeoDRICH::DD4hep_to_IRT() {

  // begin envelope
  /* FIXME: have no connection to GEANT G4LogicalVolume pointers; however all is needed
   * is to make them unique so that std::map work internally; resort to using integers,
   * who cares; material pointer can seemingly be '0', and effective refractive index
   * for all radiators will be assigned at the end by hand; FIXME: should assign it on
   * per-photon basis, at birth, like standalone GEANT code does;
   */
  auto nSectors              = m_det->constant<int>("DRICH_num_sectors");
  auto vesselZmin            = m_det->constant<double>("DRICH_zmin") / dd4hep::mm;
  auto vesselWindowThickness = m_det->constant<double>("DRICH_window_thickness") / dd4hep::mm;
  auto gasvolMaterial        = m_det->constant<std::string>("DRICH_gasvol_material");
  TVector3 normX(1, 0, 0); // normal vectors
  TVector3 normY(0, -1, 0);
  m_surfEntrance =
      new FlatSurface(TVector3(0, 0, vesselZmin + vesselWindowThickness), normX, normY);
  for (int isec = 0; isec < nSectors; isec++) {
    auto* cv = m_irtDetectorCollection->SetContainerVolume(
        m_irtDetector,              // Cherenkov detector
        RadiatorName(kGas).c_str(), // name
        isec,                       // path
        (G4LogicalVolume*)nullptr,  // G4LogicalVolume (inaccessible? use an integer instead)
        nullptr,                    // G4RadiatorMaterial (inaccessible?)
        m_surfEntrance              // surface
    );
    cv->SetAlternativeMaterialName(gasvolMaterial.c_str());
  }

  // photon detector
  // - FIXME: args (G4Solid,G4Material) inaccessible?
  auto cellMask       = uint64_t(std::stoull(m_det->constant<std::string>("DRICH_cell_mask")));
  m_irtPhotonDetector = new CherenkovPhotonDetector(nullptr, nullptr);
  m_irtDetector->SetReadoutCellMask(cellMask);
  m_irtDetectorCollection->AddPhotonDetector(m_irtDetector,      // Cherenkov detector
                                             nullptr,            // G4LogicalVolume (inaccessible?)
                                             m_irtPhotonDetector // photon detector
  );
  m_log->debug("cellMask = {:#X}", cellMask);

  // aerogel + filter
  /* AddFlatRadiator will create a pair of flat refractive surfaces internally;
   * FIXME: should make a small gas gap at the upstream end of the gas volume;
   * FIXME: do we need a sector loop?
   * FIXME: airgap radiator?
   */
  auto aerogelZpos      = m_det->constant<double>("DRICH_aerogel_zpos") / dd4hep::mm;
  auto aerogelThickness = m_det->constant<double>("DRICH_aerogel_thickness") / dd4hep::mm;
  auto aerogelMaterial  = m_det->constant<std::string>("DRICH_aerogel_material");
  auto filterZpos       = m_det->constant<double>("DRICH_filter_zpos") / dd4hep::mm;
  auto filterThickness  = m_det->constant<double>("DRICH_filter_thickness") / dd4hep::mm;
  auto filterMaterial   = m_det->constant<std::string>("DRICH_filter_material");
  m_aerogelFlatSurface  = new FlatSurface(TVector3(0, 0, aerogelZpos), normX, normY);
  m_filterFlatSurface   = new FlatSurface(TVector3(0, 0, filterZpos), normX, normY);
  for (int isec = 0; isec < nSectors; isec++) {
    auto* aerogelFlatRadiator = m_irtDetectorCollection->AddFlatRadiator(
        m_irtDetector,                  // Cherenkov detector
        RadiatorName(kAerogel).c_str(), // name
        isec,                           // path
        (G4LogicalVolume*)(0x1),        // G4LogicalVolume (inaccessible? use an integer instead)
        nullptr,                        // G4RadiatorMaterial
        m_aerogelFlatSurface,           // surface
        aerogelThickness                // surface thickness
    );
    auto* filterFlatRadiator = m_irtDetectorCollection->AddFlatRadiator(
        m_irtDetector,           // Cherenkov detector
        "Filter",                // name
        isec,                    // path
        (G4LogicalVolume*)(0x2), // G4LogicalVolume (inaccessible? use an integer instead)
        nullptr,                 // G4RadiatorMaterial
        m_filterFlatSurface,     // surface
        filterThickness          // surface thickness
    );
    aerogelFlatRadiator->SetAlternativeMaterialName(aerogelMaterial.c_str());
    filterFlatRadiator->SetAlternativeMaterialName(filterMaterial.c_str());
  }
  m_log->debug("aerogelZpos = {:f} mm", aerogelZpos);
  m_log->debug("filterZpos  = {:f} mm", filterZpos);
  m_log->debug("aerogel thickness = {:f} mm", aerogelThickness);
  m_log->debug("filter thickness  = {:f} mm", filterThickness);

  // sector loop
  for (int isec = 0; isec < nSectors; isec++) {
    std::string secName = "sec" + std::to_string(isec);
    // mirrors
    auto mirrorRadius = m_det->constant<double>("DRICH_mirror_radius") / dd4hep::mm;
    dd4hep::Position mirrorCenter(
        m_det->constant<double>("DRICH_mirror_center_x_" + secName) / dd4hep::mm,
        m_det->constant<double>("DRICH_mirror_center_y_" + secName) / dd4hep::mm,
        m_det->constant<double>("DRICH_mirror_center_z_" + secName) / dd4hep::mm);
    m_mirrorSphericalSurface = new SphericalSurface(
        TVector3(mirrorCenter.x(), mirrorCenter.y(), mirrorCenter.z()), mirrorRadius);
    m_mirrorOpticalBoundary =
        new OpticalBoundary(m_irtDetector->GetContainerVolume(), // CherenkovRadiator radiator
                            m_mirrorSphericalSurface,            // surface
                            false                                // bool refractive
        );
    m_irtDetector->AddOpticalBoundary(isec, m_mirrorOpticalBoundary);
    m_log->debug("");
    m_log->debug("  SECTOR {:d} MIRROR:", isec);
    m_log->debug("    mirror x = {:f} mm", mirrorCenter.x());
    m_log->debug("    mirror y = {:f} mm", mirrorCenter.y());
    m_log->debug("    mirror z = {:f} mm", mirrorCenter.z());
    m_log->debug("    mirror R = {:f} mm", mirrorRadius);

    // complete the radiator volume description; this is the rear side of the container gas volume
    auto* rad = m_irtDetector->GetRadiator(RadiatorName(kGas).c_str());
    if (rad != nullptr) {
      rad->m_Borders[isec].second = m_mirrorSphericalSurface;
    } else {
      throw std::runtime_error("Gas radiator not built in IrtGeo");
    }

    // sensor modules: search the detector tree for sensors for this sector
    m_log->trace("  SENSORS:");
    m_log->trace(
        "--------------------------------------------------------------------------------------");
    m_log->trace(
        "name ID sector   pos_x pos_y pos_z   normX_x normX_y normX_z   normY_x normY_y normY_z");
    m_log->trace(
        "--------------------------------------------------------------------------------------");
    auto sensorThickness = m_det->constant<double>("DRICH_sensor_thickness") / dd4hep::mm;
    auto sensorSize      = m_det->constant<double>("DRICH_sensor_size") / dd4hep::mm;
    for (auto const& [de_name, detSensor] : m_detRich.children()) {
      if (de_name.find("sensor_de_" + secName) != std::string::npos) {

        // get sensor info
        const auto sensorID       = detSensor.id();
        auto* const detSensorPars = detSensor.extension<dd4hep::rec::VariantParameters>(true);
        if (detSensorPars == nullptr) {
          throw std::runtime_error(
              fmt::format("sensor '{}' does not have VariantParameters", de_name));
        }
        // - sensor surface position
        auto posSensor =
            GetVectorFromVariantParameters<dd4hep::Position>(detSensorPars, "pos") / dd4hep::mm;
        // - sensor orientation
        auto normXdir = GetVectorFromVariantParameters<dd4hep::Direction>(detSensorPars, "normX");
        auto normYdir = GetVectorFromVariantParameters<dd4hep::Direction>(detSensorPars, "normY");
        auto normZdir = normXdir.Cross(normYdir); // sensor surface normal
        // - surface offset, used to convert sensor volume centroid to sensor surface centroid
        auto surfaceOffset = normZdir.Unit() * (0.5 * sensorThickness);

        // add sensor info to `m_sensor_info` map
        richgeo::Sensor sensor_info;
        sensor_info.size             = sensorSize;
        sensor_info.surface_centroid = posSensor;
        sensor_info.surface_offset   = surfaceOffset;
        m_sensor_info.insert({sensorID, sensor_info});
        // create the optical surface
        m_sensorFlatSurface = new FlatSurface(TVector3(posSensor.x(), posSensor.y(), posSensor.z()),
                                              TVector3(normXdir.x(), normXdir.y(), normXdir.z()),
                                              TVector3(normYdir.x(), normYdir.y(), normYdir.z()));
        m_irtDetector->CreatePhotonDetectorInstance(isec,                // sector
                                                    m_irtPhotonDetector, // CherenkovPhotonDetector
                                                    sensorID,            // copy number
                                                    m_sensorFlatSurface  // surface
        );
        m_log->trace("{} {:#X} {}   {:5.2f} {:5.2f} {:5.2f}   {:5.2f} {:5.2f} {:5.2f}   {:5.2f} "
                     "{:5.2f} {:5.2f}",
                     de_name, sensorID, isec, posSensor.x(), posSensor.y(), posSensor.z(),
                     normXdir.x(), normXdir.y(), normXdir.z(), normYdir.x(), normYdir.y(),
                     normYdir.z());
      }
    } // search for sensors

  } // sector loop

  // set reference refractive indices // NOTE: numbers may be overridden externally
  std::map<const std::string, double> rIndices;
  rIndices.insert({RadiatorName(kGas), 1.00076});
  rIndices.insert({RadiatorName(kAerogel), 1.0190});
  rIndices.insert({"Filter", 1.5017});
  for (auto const& [rName, rIndex] : rIndices) {
    auto* rad = m_irtDetector->GetRadiator(rName.c_str());
    if (rad != nullptr) {
      rad->SetReferenceRefractiveIndex(rIndex);
    }
  }

  // set refractive index table
  SetRefractiveIndexTable();

  // define the `cell ID -> pixel position` converter
  SetReadoutIDToPositionLambda();
}
TVector3 richgeo::IrtGeoDRICH::GetSensorSurfaceNorm(CellIDType id) {
  TVector3 sensorNorm;
  auto cellMask       = uint64_t(std::stoull(m_det->constant<std::string>("DRICH_cell_mask")));
  auto sensor_info    = this->m_sensor_info;
  auto sID            = id & cellMask;
  auto sensor_info_it = sensor_info.find(sID);
  if (sensor_info_it != sensor_info.end()) {
    auto sensor_obj = sensor_info_it->second;
    auto normZdir   = sensor_obj.surface_offset.Unit();
    sensorNorm.SetX(static_cast<double>(normZdir.x()));
    sensorNorm.SetY(static_cast<double>(normZdir.y()));
    sensorNorm.SetZ(static_cast<double>(normZdir.z()));
  } else {
    m_log->error("Cannot find sensor {} in IrtGeoDRICH::GetSensorSurface", id);
    throw std::runtime_error("sensor not found in IrtGeoDRIC::GetSensorSurfaceNormal");
  }
  return sensorNorm;
}
// destructor
richgeo::IrtGeoDRICH::~IrtGeoDRICH() {
  delete m_surfEntrance;
  delete m_irtPhotonDetector;
  delete m_aerogelFlatSurface;
  delete m_filterFlatSurface;
  delete m_mirrorSphericalSurface;
  delete m_mirrorOpticalBoundary;
  delete m_sensorFlatSurface;
}
