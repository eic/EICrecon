// Copyright (C) 2022, 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "IrtGeoPFRICH.h"

#include <DD4hep/DetElement.h>
#include <DD4hep/Fields.h>
#include <DD4hep/Volumes.h>
#include <Evaluator/DD4hepUnits.h>
#include <IRT/CherenkovDetector.h>
#include <IRT/CherenkovDetectorCollection.h>
#include <IRT/CherenkovRadiator.h>
#include <IRT/G4Object.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <RtypesCore.h>
#include <TGeoNode.h>
#include <TRef.h>
#include <TVector3.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <cmath>
#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>
#include <utility>

#include "services/geometry/richgeo/RichGeo.h"

void richgeo::IrtGeoPFRICH::DD4hep_to_IRT() {

  // begin envelope
  /* FIXME: have no connection to GEANT G4LogicalVolume pointers; however all is needed
   * is to make them unique so that std::map work internally; resort to using integers,
   * who cares; material pointer can seemingly be '0', and effective refractive index
   * for all radiators will be assigned at the end by hand; FIXME: should assign it on
   * per-photon basis, at birth, like standalone GEANT code does;
   */
  auto vesselZmin     = m_det->constant<double>("PFRICH_zmin") / dd4hep::mm;
  auto gasvolMaterial = m_det->constant<std::string>("PFRICH_gasvol_material");
  TVector3 normX(1, 0, 0); // normal vectors
  TVector3 normY(0, 1, 0);
  m_surfEntrance = new FlatSurface(TVector3(0, 0, vesselZmin), normX, normY);
  auto* cv       = m_irtDetectorCollection->SetContainerVolume(
      m_irtDetector,              // Cherenkov detector
      RadiatorName(kGas).c_str(), // name
      0,                          // path
      (G4LogicalVolume*)nullptr,  // G4LogicalVolume (inaccessible? use an integer instead)
      nullptr,                    // G4RadiatorMaterial (inaccessible?)
      m_surfEntrance              // surface
  );
  cv->SetAlternativeMaterialName(gasvolMaterial.c_str());

  // photon detector
  // - FIXME: args (G4Solid,G4Material) inaccessible?
  auto cellMask       = uint64_t(std::stoull(m_det->constant<std::string>("PFRICH_cell_mask")));
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
   */
  auto aerogelZpos          = m_det->constant<double>("PFRICH_aerogel_zpos") / dd4hep::mm;
  auto aerogelThickness     = m_det->constant<double>("PFRICH_aerogel_thickness") / dd4hep::mm;
  auto aerogelMaterial      = m_det->constant<std::string>("PFRICH_aerogel_material");
  auto filterZpos           = m_det->constant<double>("PFRICH_filter_zpos") / dd4hep::mm;
  auto filterThickness      = m_det->constant<double>("PFRICH_filter_thickness") / dd4hep::mm;
  auto filterMaterial       = m_det->constant<std::string>("PFRICH_filter_material");
  m_aerogelFlatSurface      = new FlatSurface(TVector3(0, 0, aerogelZpos), normX, normY);
  m_filterFlatSurface       = new FlatSurface(TVector3(0, 0, filterZpos), normX, normY);
  auto* aerogelFlatRadiator = m_irtDetectorCollection->AddFlatRadiator(
      m_irtDetector,                  // Cherenkov detector
      RadiatorName(kAerogel).c_str(), // name
      0,                              // path
      (G4LogicalVolume*)(0x1),        // G4LogicalVolume (inaccessible? use an integer instead)
      nullptr,                        // G4RadiatorMaterial
      m_aerogelFlatSurface,           // surface
      aerogelThickness                // surface thickness
  );
  auto* filterFlatRadiator = m_irtDetectorCollection->AddFlatRadiator(
      m_irtDetector,           // Cherenkov detector
      "Filter",                // name
      0,                       // path
      (G4LogicalVolume*)(0x2), // G4LogicalVolume (inaccessible? use an integer instead)
      nullptr,                 // G4RadiatorMaterial
      m_filterFlatSurface,     // surface
      filterThickness          // surface thickness
  );
  aerogelFlatRadiator->SetAlternativeMaterialName(aerogelMaterial.c_str());
  filterFlatRadiator->SetAlternativeMaterialName(filterMaterial.c_str());
  m_log->debug("aerogelZpos = {:f} mm", aerogelZpos);
  m_log->debug("filterZpos  = {:f} mm", filterZpos);
  m_log->debug("aerogel thickness = {:f} mm", aerogelThickness);
  m_log->debug("filter thickness  = {:f} mm", filterThickness);

  // sensor modules: search the detector tree for sensors
  auto sensorThickness = m_det->constant<double>("PFRICH_sensor_thickness") / dd4hep::mm;
  auto sensorSize      = m_det->constant<double>("PFRICH_sensor_size") / dd4hep::mm;
  bool firstSensor     = true;
  for (auto const& [de_name, detSensor] : m_detRich.children()) {
    if (de_name.find("sensor_de") != std::string::npos) {

      // get sensor info
      auto imod = detSensor.id();
      // - get sensor centroid position
      auto pvSensor  = detSensor.placement();
      auto posSensor = (1 / dd4hep::mm) * (m_posRich + pvSensor.position());
      // - get sensor surface position
      dd4hep::Direction sensorNorm(
          0, 0,
          1); // FIXME: generalize; this assumes planar layout, with norm along +z axis (toward IP)
      auto surfaceOffset    = sensorNorm.Unit() * (0.5 * sensorThickness);
      auto posSensorSurface = posSensor + surfaceOffset;
      // - add to `m_sensor_info` map
      richgeo::Sensor sensor_info;
      sensor_info.size             = sensorSize;
      sensor_info.surface_centroid = posSensorSurface;
      sensor_info.surface_offset   = surfaceOffset;
      m_sensor_info.insert({imod, sensor_info});
      // - get surface normal and in-plane vectors
      double sensorLocalNormX[3] = {1.0, 0.0, 0.0};
      double sensorLocalNormY[3] = {0.0, 1.0, 0.0};
      double sensorGlobalNormX[3];
      double sensorGlobalNormY[3];
      pvSensor.ptr()->LocalToMasterVect(
          static_cast<const Double_t*>(sensorLocalNormX),
          static_cast<Double_t*>(
              sensorGlobalNormX)); // ignore vessel transformation, since it is a pure translation
      pvSensor.ptr()->LocalToMasterVect(static_cast<const Double_t*>(sensorLocalNormY),
                                        static_cast<Double_t*>(sensorGlobalNormY));

      // validate sensor position and normal
      // - test normal vectors
      dd4hep::Direction normXdir;
      dd4hep::Direction normYdir;
      normXdir.SetCoordinates(static_cast<const Double_t*>(sensorGlobalNormX));
      normYdir.SetCoordinates(static_cast<const Double_t*>(sensorGlobalNormY));
      auto normZdir =
          normXdir.Cross(normYdir); // sensor surface normal, given derived GlobalNormX,Y
      auto testOrtho  = normXdir.Dot(normYdir); // should be zero, if normX and normY are orthogonal
      auto testRadial = sensorNorm.Cross(normZdir)
                            .Mag2(); // should be zero, if sensor surface normal is as expected
      if (std::abs(testOrtho) > 1e-6 || std::abs(testRadial) > 1e-6) {
        m_log->error(
            "sensor normal is wrong: normX.normY = {:f}   |sensorNorm x normZdir|^2 = {:f}",
            testOrtho, testRadial);
        return;
      }

      // create the optical surface
      m_sensorFlatSurface = new FlatSurface(
          TVector3(posSensorSurface.x(), posSensorSurface.y(), posSensorSurface.z()),
          TVector3(sensorGlobalNormX), TVector3(sensorGlobalNormY));
      m_irtDetector->CreatePhotonDetectorInstance(0,                   // sector
                                                  m_irtPhotonDetector, // CherenkovPhotonDetector
                                                  imod,                // copy number
                                                  m_sensorFlatSurface  // surface
      );
      m_log->trace("sensor: id={:#08X} pos=({:5.2f}, {:5.2f}, {:5.2f}) normX=({:5.2f}, {:5.2f}, "
                   "{:5.2f}) normY=({:5.2f}, {:5.2f}, {:5.2f})",
                   imod, posSensorSurface.x(), posSensorSurface.y(), posSensorSurface.z(),
                   normXdir.x(), normXdir.y(), normXdir.z(), normYdir.x(), normYdir.y(),
                   normYdir.z());

      // complete the radiator volume description; this is the rear side of the container gas volume
      // Yes, since there are no mirrors in this detector, just close the gas radiator volume by hand (once),
      // assuming that all the sensors will be sitting at roughly the same location along the beam line anyway;
      if (firstSensor) {
        m_irtDetector->GetRadiator(RadiatorName(kGas).c_str())->m_Borders[0].second =
            dynamic_cast<ParametricSurface*>(m_sensorFlatSurface);
        firstSensor = false;
      }

    } // if sensor found
  } // search for sensors

  // set reference refractive indices // NOTE: numbers may be overridden externally
  std::map<const char*, double> rIndices;
  rIndices.insert({RadiatorName(kGas).c_str(), 1.0013});
  rIndices.insert({RadiatorName(kAerogel).c_str(), 1.0190});
  rIndices.insert({"Filter", 1.5017});
  for (auto const& [rName, rIndex] : rIndices) {
    auto* rad = m_irtDetector->GetRadiator(rName);
    if (rad != nullptr) {
      rad->SetReferenceRefractiveIndex(rIndex);
    }
  }

  // set refractive index table
  SetRefractiveIndexTable();

  // define the `cell ID -> pixel position` converter
  SetReadoutIDToPositionLambda();
}

// destructor
richgeo::IrtGeoPFRICH::~IrtGeoPFRICH() {
  delete m_surfEntrance;
  delete m_irtPhotonDetector;
  delete m_aerogelFlatSurface;
  delete m_filterFlatSurface;
  delete m_sensorFlatSurface;
}
