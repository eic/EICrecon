// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include "IrtGeoPFRICH.h"

void richgeo::IrtGeoPFRICH::DD4hep_to_IRT() {

  // begin envelope
  /* FIXME: have no connection to GEANT G4LogicalVolume pointers; however all is needed
   * is to make them unique so that std::map work internally; resort to using integers,
   * who cares; material pointer can seemingly be '0', and effective refractive index
   * for all radiators will be assigned at the end by hand; FIXME: should assign it on
   * per-photon basis, at birth, like standalone GEANT code does;
   */
  auto vesselZmin     = m_det->constant<double>("PFRICH_zmin");
  auto gasvolMaterial = m_det->constant<std::string>("PFRICH_gasvol_material");
  TVector3 normX(1, 0,  0); // normal vectors
  TVector3 normY(0, 1, 0);
  auto surfEntrance = new FlatSurface((1 / dd4hep::mm) * TVector3(0, 0, vesselZmin), normX, normY);
  auto cv = m_irtDetectorCollection->SetContainerVolume(
      m_irtDetector,              // Cherenkov detector
      RadiatorName(kGas).c_str(), // name
      0,                          // path
      (G4LogicalVolume*)(0x0),    // G4LogicalVolume (inaccessible? use an integer instead)
      nullptr,                    // G4RadiatorMaterial (inaccessible?)
      surfEntrance                // surface
      );
  cv->SetAlternativeMaterialName(gasvolMaterial.c_str());

  // photon detector
  // - FIXME: args (G4Solid,G4Material) inaccessible?
  auto cellMask = uint64_t(std::stoull(m_det->constant<std::string>("PFRICH_cell_mask")));
  CherenkovPhotonDetector* irtPhotonDetector = new CherenkovPhotonDetector(nullptr, nullptr);
  m_irtDetector->SetReadoutCellMask(cellMask);
  m_irtDetectorCollection->AddPhotonDetector(
      m_irtDetector,    // Cherenkov detector
      nullptr,          // G4LogicalVolume (inaccessible?)
      irtPhotonDetector // photon detector
      );
  m_log.PrintLog("cellMask = {:#X}", cellMask);

  // aerogel + filter
  /* AddFlatRadiator will create a pair of flat refractive surfaces internally;
   * FIXME: should make a small gas gap at the upstream end of the gas volume;
   */
  auto aerogelZpos        = m_det->constant<double>("PFRICH_aerogel_zpos");
  auto aerogelThickness   = m_det->constant<double>("PFRICH_aerogel_thickness");
  auto aerogelMaterial    = m_det->constant<std::string>("PFRICH_aerogel_material");
  auto filterZpos         = m_det->constant<double>("PFRICH_filter_zpos");
  auto filterThickness    = m_det->constant<double>("PFRICH_filter_thickness");
  auto filterMaterial     = m_det->constant<std::string>("PFRICH_filter_material");
  auto aerogelFlatSurface = new FlatSurface((1 / dd4hep::mm) * TVector3(0, 0, aerogelZpos), normX, normY);
  auto filterFlatSurface  = new FlatSurface((1 / dd4hep::mm) * TVector3(0, 0, filterZpos),  normX, normY);
  auto aerogelFlatRadiator = m_irtDetectorCollection->AddFlatRadiator(
      m_irtDetector,                  // Cherenkov detector
      RadiatorName(kAerogel).c_str(), // name
      0,                              // path
      (G4LogicalVolume*)(0x1),        // G4LogicalVolume (inaccessible? use an integer instead)
      nullptr,                        // G4RadiatorMaterial
      aerogelFlatSurface,             // surface
      aerogelThickness / dd4hep::mm   // surface thickness
      );
  auto filterFlatRadiator = m_irtDetectorCollection->AddFlatRadiator(
      m_irtDetector,           // Cherenkov detector
      "Filter",                // name
      0,                       // path
      (G4LogicalVolume*)(0x2), // G4LogicalVolume (inaccessible? use an integer instead)
      nullptr,                 // G4RadiatorMaterial
      filterFlatSurface,       // surface
      filterThickness / dd4hep::mm // surface thickness
      );
  aerogelFlatRadiator->SetAlternativeMaterialName(aerogelMaterial.c_str());
  filterFlatRadiator->SetAlternativeMaterialName(filterMaterial.c_str());
  m_log.PrintLog("aerogelZpos = {:f} cm", aerogelZpos);
  m_log.PrintLog("filterZpos  = {:f} cm", filterZpos);
  m_log.PrintLog("aerogel thickness = {:f} cm", aerogelThickness);
  m_log.PrintLog("filter thickness  = {:f} cm", filterThickness);

  // sensor modules: search the detector tree for sensors
  auto sensorThickness  = m_det->constant<double>("PFRICH_sensor_thickness");
  bool firstSensor = true;
  for(auto const& [de_name, detSensor] : m_detRich.children()) {
    if(de_name.find("sensor_de")!=std::string::npos) {

      // get sensor info
      auto imod = detSensor.id();
      // - get sensor centroid position
      auto pvSensor  = detSensor.placement();
      auto posSensor = m_posRich + pvSensor.position();
      // - get sensor surface position
      dd4hep::Direction sensorNorm(0,0,1); // FIXME: generalize; this assumes planar layout, with norm along +z axis (toward IP)
      auto posSensorSurface = posSensor + (sensorNorm.Unit() * (0.5*sensorThickness));
      // - get surface normal and in-plane vectors
      double sensorLocalNormX[3] = {1.0, 0.0, 0.0};
      double sensorLocalNormY[3] = {0.0, 1.0, 0.0};
      double sensorGlobalNormX[3], sensorGlobalNormY[3];
      pvSensor.ptr()->LocalToMasterVect(sensorLocalNormX, sensorGlobalNormX); // ignore vessel transformation, since it is a pure translation
      pvSensor.ptr()->LocalToMasterVect(sensorLocalNormY, sensorGlobalNormY);

      // validate sensor position and normal
      // - test normal vectors
      dd4hep::Direction normXdir, normYdir;
      normXdir.SetCoordinates(sensorGlobalNormX);
      normYdir.SetCoordinates(sensorGlobalNormY);
      auto normZdir   = normXdir.Cross(normYdir);         // sensor surface normal, given derived GlobalNormX,Y
      auto testOrtho  = normXdir.Dot(normYdir);           // should be zero, if normX and normY are orthogonal
      auto testRadial = sensorNorm.Cross(normZdir).Mag2(); // should be zero, if sensor surface normal is as expected
      if(abs(testOrtho)>1e-6 || abs(testRadial)>1e-6) {
        m_log.PrintError(
            "sensor normal is wrong: normX.normY = {:f}   |sensorNorm x normZdir|^2 = {:f}",
            testOrtho,
            testRadial
            );
        return;
      }

      // create the optical surface
      auto sensorFlatSurface = new FlatSurface(
          (1 / dd4hep::mm) * TVector3(posSensorSurface.x(), posSensorSurface.y(), posSensorSurface.z()),
          TVector3(sensorGlobalNormX),
          TVector3(sensorGlobalNormY)
          );
      m_irtDetector->CreatePhotonDetectorInstance(
          0,                 // sector
          irtPhotonDetector, // CherenkovPhotonDetector
          imod,              // copy number
          sensorFlatSurface  // surface
          );
      m_log.PrintLog(
          "sensor: id={:#08X} pos=({:5.2f}, {:5.2f}, {:5.2f}) normX=({:5.2f}, {:5.2f}, {:5.2f}) normY=({:5.2f}, {:5.2f}, {:5.2f})",
          imod,
          posSensorSurface.x(), posSensorSurface.y(), posSensorSurface.z(),
          normXdir.x(),  normXdir.y(),  normXdir.z(),
          normYdir.x(),  normYdir.y(),  normYdir.z()
          );

      // complete the radiator volume description; this is the rear side of the container gas volume
      // Yes, since there are no mirrors in this detector, just close the gas radiator volume by hand (once),
      // assuming that all the sensors will be sitting at roughly the same location along the beam line anyway;
      if(firstSensor) {
        m_irtDetector->GetRadiator(RadiatorName(kGas).c_str())->m_Borders[0].second = dynamic_cast<ParametricSurface*>(sensorFlatSurface);
        firstSensor = false;
      }

    } // if sensor found
  } // search for sensors

  // set refractive indices
  // FIXME: are these (weighted) averages? can we automate this? We should avoid hard-coded numbers here!
  std::map<const char*, double> rIndices;
  rIndices.insert({RadiatorName(kGas).c_str(),     1.0013});
  rIndices.insert({RadiatorName(kAerogel).c_str(), 1.0190});
  rIndices.insert({"Filter",                       1.5017});
  for (auto const& [rName, rIndex] : rIndices) {
    auto rad = m_irtDetector->GetRadiator(rName);
    if (rad)
      rad->SetReferenceRefractiveIndex(rIndex);
  }
}
