// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include "IrtGeoDRICH.h"

void richgeo::IrtGeoDRICH::DD4hep_to_IRT() {

  // begin envelope
  /* FIXME: have no connection to GEANT G4LogicalVolume pointers; however all is needed
   * is to make them unique so that std::map work internally; resort to using integers,
   * who cares; material pointer can seemingly be '0', and effective refractive index
   * for all radiators will be assigned at the end by hand; FIXME: should assign it on
   * per-photon basis, at birth, like standalone GEANT code does;
   */
  auto nSectors       = m_det->constant<int>("DRICH_num_sectors");
  auto vesselZmin     = m_det->constant<double>("DRICH_zmin");
  auto gasvolMaterial = m_det->constant<std::string>("DRICH_gasvol_material");
  TVector3 normX(1, 0,  0); // normal vectors
  TVector3 normY(0, -1, 0);
  auto surfEntrance = new FlatSurface((1 / dd4hep::mm) * TVector3(0, 0, vesselZmin), normX, normY);
  for (int isec=0; isec<nSectors; isec++) {
    auto cv = m_irtDetectorCollection->SetContainerVolume(
        m_irtDetector,              // Cherenkov detector
        RadiatorName(kGas).c_str(), // name
        isec,                       // path
        (G4LogicalVolume*)(0x0),    // G4LogicalVolume (inaccessible? use an integer instead)
        nullptr,                    // G4RadiatorMaterial (inaccessible?)
        surfEntrance                // surface
        );
    cv->SetAlternativeMaterialName(gasvolMaterial.c_str());
  }

  // photon detector
  // - FIXME: args (G4Solid,G4Material) inaccessible?
  auto cellMask = uint64_t(std::stoull(m_det->constant<std::string>("DRICH_cell_mask")));
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
   * FIXME: do we need a sector loop?
   * FIXME: airgap radiator?
   */
  auto aerogelZpos        = m_det->constant<double>("DRICH_aerogel_zpos");
  auto aerogelThickness   = m_det->constant<double>("DRICH_aerogel_thickness");
  auto aerogelMaterial    = m_det->constant<std::string>("DRICH_aerogel_material");
  auto filterZpos         = m_det->constant<double>("DRICH_filter_zpos");
  auto filterThickness    = m_det->constant<double>("DRICH_filter_thickness");
  auto filterMaterial     = m_det->constant<std::string>("DRICH_filter_material");
  auto aerogelFlatSurface = new FlatSurface((1 / dd4hep::mm) * TVector3(0, 0, aerogelZpos), normX, normY);
  auto filterFlatSurface  = new FlatSurface((1 / dd4hep::mm) * TVector3(0, 0, filterZpos),  normX, normY);
  for (int isec = 0; isec < nSectors; isec++) {
    auto aerogelFlatRadiator = m_irtDetectorCollection->AddFlatRadiator(
        m_irtDetector,                  // Cherenkov detector
        RadiatorName(kAerogel).c_str(), // name
        isec,                           // path
        (G4LogicalVolume*)(0x1),        // G4LogicalVolume (inaccessible? use an integer instead)
        nullptr,                        // G4RadiatorMaterial
        aerogelFlatSurface,             // surface
        aerogelThickness / dd4hep::mm   // surface thickness
        );
    auto filterFlatRadiator = m_irtDetectorCollection->AddFlatRadiator(
        m_irtDetector,           // Cherenkov detector
        "Filter",                // name
        isec,                    // path
        (G4LogicalVolume*)(0x2), // G4LogicalVolume (inaccessible? use an integer instead)
        nullptr,                 // G4RadiatorMaterial
        filterFlatSurface,       // surface
        filterThickness / dd4hep::mm // surface thickness
        );
    aerogelFlatRadiator->SetAlternativeMaterialName(aerogelMaterial.c_str());
    filterFlatRadiator->SetAlternativeMaterialName(filterMaterial.c_str());
  }
  m_log.PrintLog("aerogelZpos = {:f} cm", aerogelZpos);
  m_log.PrintLog("filterZpos  = {:f} cm", filterZpos);
  m_log.PrintLog("aerogel thickness = {:f} cm", aerogelThickness);
  m_log.PrintLog("filter thickness  = {:f} cm", filterThickness);

  // sector loop
  for (int isec = 0; isec < nSectors; isec++) {
    std::string secName = "sec" + std::to_string(isec);

    // mirrors
    auto mirrorRadius = m_det->constant<double>("DRICH_mirror_radius");
    dd4hep::Position mirrorCenter(
      m_det->constant<double>("DRICH_mirror_center_x_"+secName),
      m_det->constant<double>("DRICH_mirror_center_y_"+secName),
      m_det->constant<double>("DRICH_mirror_center_z_"+secName)
      );
    auto mirrorSphericalSurface  = new SphericalSurface(
        (1 / dd4hep::mm) * TVector3(mirrorCenter.x(), mirrorCenter.y(), mirrorCenter.z()), mirrorRadius / dd4hep::mm);
    auto mirrorOpticalBoundary = new OpticalBoundary(
        m_irtDetector->GetContainerVolume(), // CherenkovRadiator radiator
        mirrorSphericalSurface,            // surface
        false                              // bool refractive
        );
    m_irtDetector->AddOpticalBoundary(isec, mirrorOpticalBoundary);
    m_log.PrintLog("");
    m_log.PrintLog("  SECTOR {:d} MIRROR:", isec);
    m_log.PrintLog("    mirror x = {:f} cm", mirrorCenter.x());
    m_log.PrintLog("    mirror y = {:f} cm", mirrorCenter.y());
    m_log.PrintLog("    mirror z = {:f} cm", mirrorCenter.z());
    m_log.PrintLog("    mirror R = {:f} cm", mirrorRadius);

    // complete the radiator volume description; this is the rear side of the container gas volume
    m_irtDetector->GetRadiator(RadiatorName(kGas).c_str())->m_Borders[isec].second = mirrorSphericalSurface;

    // sensor sphere (only used for validation of sensor normals)
    auto sensorSphRadius  = m_det->constant<double>("DRICH_sensor_sph_radius");
    auto sensorThickness  = m_det->constant<double>("DRICH_sensor_thickness");
    dd4hep::Position sensorSphCenter(
      m_det->constant<double>("DRICH_sensor_sph_center_x_"+secName),
      m_det->constant<double>("DRICH_sensor_sph_center_y_"+secName),
      m_det->constant<double>("DRICH_sensor_sph_center_z_"+secName)
      );
    m_log.PrintLog("  SECTOR {:d} SENSOR SPHERE:", isec);
    m_log.PrintLog("    sphere x = {:f} cm", sensorSphCenter.x());
    m_log.PrintLog("    sphere y = {:f} cm", sensorSphCenter.y());
    m_log.PrintLog("    sphere z = {:f} cm", sensorSphCenter.z());
    m_log.PrintLog("    sphere R = {:f} cm", sensorSphRadius);

    // sensor modules: search the detector tree for sensors for this sector
    for(auto const& [de_name, detSensor] : m_detRich.children()) {
      if(de_name.find("sensor_de_"+secName)!=std::string::npos) {

        // get sensor info
        auto imodsec = detSensor.id();
        // - get sensor centroid position
        auto pvSensor  = detSensor.placement();
        auto posSensor = m_posRich + pvSensor.position();
        // - get sensor surface position
        dd4hep::Direction radialDir   = posSensor - sensorSphCenter; // sensor sphere radius direction
        auto posSensorSurface = posSensor + (radialDir.Unit() * (0.5*sensorThickness));
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
        auto normZdir   = normXdir.Cross(normYdir);         // sensor surface normal
        auto testOrtho  = normXdir.Dot(normYdir);           // should be zero, if normX and normY are orthogonal
        auto testRadial = radialDir.Cross(normZdir).Mag2(); // should be zero, if sensor surface normal is parallel to sensor sphere radius
        if(abs(testOrtho)>1e-6 || abs(testRadial)>1e-6) {
          m_log.PrintError(
              "sensor normal is wrong: normX.normY = {:f}   |radialDir x normZdir|^2 = {:f}",
              testOrtho,
              testRadial
              );
          return;
        }
        // - test sensor positioning
        auto distSensor2center = sqrt((posSensorSurface-sensorSphCenter).Mag2()); // distance between sensor sphere center and sensor surface position
        auto testDist          = abs(distSensor2center-sensorSphRadius);          // should be zero, if sensor position w.r.t. sensor sphere center is correct
        if(abs(testDist)>1e-6) {
          m_log.PrintError(
              "sensor positioning is wrong: dist(sensor, sphere_center) = {:f},  sphere_radius = {:f},  sensor_thickness = {:f},  |diff| = {:g}\n",
              distSensor2center,
              sensorSphRadius,
              sensorThickness,
              testDist
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
            isec,              // sector
            irtPhotonDetector, // CherenkovPhotonDetector
            imodsec,           // copy number
            sensorFlatSurface  // surface
            );
        /*
        m_log.PrintLog(
            "sensor: id={:#08X} pos=({:5.2f}, {:5.2f}, {:5.2f}) normX=({:5.2f}, {:5.2f}, {:5.2f}) normY=({:5.2f}, {:5.2f}, {:5.2f})",
            imodsec,
            posSensorSurface.x(), posSensorSurface.y(), posSensorSurface.z(),
            normXdir.x(),  normXdir.y(),  normXdir.z(),
            normYdir.x(),  normYdir.y(),  normYdir.z()
            );
            */
      }
    } // search for sensors

  } // sector loop

  // set refractive indices
  // FIXME: are these (weighted) averages? can we automate this?
  std::map<const char*, double> rIndices;
  rIndices.insert({RadiatorName(kGas).c_str(),     1.00076});
  rIndices.insert({RadiatorName(kAerogel).c_str(), 1.0190});
  rIndices.insert({"Filter",                       1.5017});
  for (auto const& [rName, rIndex] : rIndices) {
    auto rad = m_irtDetector->GetRadiator(rName);
    if (rad)
      rad->SetReferenceRefractiveIndex(rIndex);
  }
}
