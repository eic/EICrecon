// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Wouter Deconinck

#include <DD4hep/DetElement.h>
#include <DD4hep/Detector.h>
#include <DD4hep/Handle.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Objects.h>
#include <DD4hep/Readout.h>
#include <DD4hep/Segmentations.h>
#include <DD4hep/Shapes.h>
#include <DD4hep/VolumeManager.h>
#include <DD4hep/Volumes.h>
#include <DD4hep/detail/DetectorInterna.h>
#include <DD4hep/detail/SegmentationsInterna.h>
#include <DDSegmentation/CartesianGridXY.h>
#include <DDSegmentation/MultiSegmentation.h>
#include <Math/GenVector/Cartesian3D.h>
#include <Math/GenVector/DisplacementVector3D.h>
#include <TGeoMaterial.h>
#include <TGeoMedium.h>
#include <algorithms/geo.h>
#include <algorithms/interfaces/UniqueIDGenSvc.h>
#include <algorithms/random.h>
#include <algorithms/service.h>
#include <catch2/generators/catch_generators_random.hpp>
#include <catch2/interfaces/catch_interfaces_reporter.hpp>
#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>
#include <services/evaluator/EvaluatorSvc.h>
#include <services/particle/ParticleSvc.h>
#include <services/pid_lut/PIDLookupTableSvc.h>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <utility>

class algorithmsInitListener : public Catch::EventListenerBase {
public:
  using Catch::EventListenerBase::EventListenerBase;

  std::unique_ptr<const dd4hep::Detector> m_detector{nullptr};

  void testRunStarting(Catch::TestRunInfo const& /*testRunInfo*/) override {
    auto detector = dd4hep::Detector::make_unique("");
    detector->addConstant(dd4hep::Constant("MockCalorimeter_ID", "1"));
    dd4hep::Readout readout(std::string("MockCalorimeterHits"));
    dd4hep::IDDescriptor id_desc("MockCalorimeterHits", "system:8,layer:8,x:8,y:8");
    readout.setIDDescriptor(id_desc);
    detector->add(id_desc);
    detector->add(readout);

    detector->addConstant(dd4hep::Constant("MockTracker_ID", "2"));
    dd4hep::Readout readoutTracker(std::string("MockTrackerHits"));
    dd4hep::IDDescriptor id_desc_tracker("MockTrackerHits", "system:8,layer:8,x:8,y:8");
    //Create segmentation with 1x1 mm pixels
    dd4hep::Segmentation segmentation("CartesianGridXY", "TrackerHitsSeg",
                                      id_desc_tracker.decoder());
    readoutTracker.setIDDescriptor(id_desc_tracker);
    readoutTracker.setSegmentation(segmentation);
    detector->add(id_desc_tracker);
    detector->add(readoutTracker);

    dd4hep::Readout readoutSilicon(std::string("MockSiliconHits"));
    dd4hep::IDDescriptor id_desc_Silicon("MockSiliconHits",
                                         "system:8,layer:4,module:12,sensor:10,x:40:-8,y:-16");
    //Create segmentation with 1x1 mm pixels
    dd4hep::Segmentation segmentation_Silicon("CartesianGridXY", "SiliconHitsSeg",
                                              id_desc_tracker.decoder());
    readoutSilicon.setIDDescriptor(id_desc_Silicon);
    readoutSilicon.setSegmentation(segmentation_Silicon);
    detector->add(id_desc_Silicon);
    detector->add(readoutSilicon);

    // Mock MPGD readout with full geometry registered in the VolumeManager.
    // MPGDTrackerDigi requires a MultiSegmentation discriminating on "strip",
    // with CartesianGridXY sub-segmentations for p-strip (key=1) and n-strip (key=2).
    dd4hep::Readout readoutMPGD(std::string("MockMPGDHits"));
    dd4hep::IDDescriptor id_desc_mpgd(
        "MockMPGDHits", "system:8,layer:4,module:12,sensor:24:4,strip:28:4,x:32:-16,y:-16");
    {
      // Build MultiSegmentation at the DDSegmentation level, then wrap it.
      auto* multiSeg = new dd4hep::DDSegmentation::MultiSegmentation(
          "system:8,layer:4,module:12,sensor:24:4,strip:28:4,x:32:-16,y:-16");
      // Set the discriminator field to "strip" (must be done via the registered parameter).
      multiSeg->parameter("key")->setValue("strip");

      // Sub-segmentation for p-strip (strip field value = 1)
      auto* pSeg = new dd4hep::DDSegmentation::CartesianGridXY(
          "system:8,layer:4,module:12,sensor:24:4,strip:28:4,x:32:-16,y:-16");
      pSeg->setGridSizeX(1.0); // 1 mm pitch
      pSeg->setGridSizeY(1.0);
      multiSeg->addSubsegmentation(1, 1, pSeg);

      // Sub-segmentation for n-strip (strip field value = 2)
      auto* nSeg = new dd4hep::DDSegmentation::CartesianGridXY(
          "system:8,layer:4,module:12,sensor:24:4,strip:28:4,x:32:-16,y:-16");
      nSeg->setGridSizeX(1.0);
      nSeg->setGridSizeY(1.0);
      multiSeg->addSubsegmentation(2, 2, nSeg);

      // Wrap in a dd4hep::SegmentationObject and set decoder
      auto* segObj = new dd4hep::SegmentationObject(multiSeg);
      multiSeg->setDecoder(id_desc_mpgd.decoder());

      dd4hep::Segmentation segmentation_MPGD(segObj);
      readoutMPGD.setIDDescriptor(id_desc_mpgd);
      readoutMPGD.setSegmentation(segmentation_MPGD);
    }
    detector->add(id_desc_mpgd);
    detector->add(readoutMPGD);

    // World constants must be defined before Detector::init().
    detector->add(dd4hep::Constant("world_x", "1000.0"));
    detector->add(dd4hep::Constant("world_y", "1000.0"));
    detector->add(dd4hep::Constant("world_z", "1000.0"));
    // Add to actual Evaluator global state
    dd4hep::_toDictionary("world_x", "1000.0");
    dd4hep::_toDictionary("world_y", "1000.0");
    dd4hep::_toDictionary("world_z", "1000.0");

    auto* matVac = new TGeoMaterial("Vacuum", 0, 0, 0);
    auto* matAir = new TGeoMaterial("Air", 0, 0, 0);
    new TGeoMedium("Vacuum", 1, matVac);
    new TGeoMedium("Air", 2, matAir);

    detector->init();

    // SD name must match the top-level DetElement name ("MockMPGD").
    dd4hep::SensitiveDetector sd("MockMPGD", "tracker");
    sd.setReadout(readoutMPGD);
    detector->add(sd);

    // Hierarchy: world → envelope ("system") → module ("layer","module") → sensor ("strip")
    // The algorithm expects 3 subvolumes per module:
    //   strip=0 (reference/drift volume), strip=1 (p-strip), strip=2 (n-strip)
    // The strip=0 volume is the "reference subvolume" used for coordinate transforms.
    dd4hep::Box sensorShape("sensor_shape", 5.0, 5.0, 0.025);
    dd4hep::Volume sensorVol("MockMPGDSensor", sensorShape, detector->air());
    sensorVol.setSensitiveDetector(sd);

    // Reference (drift) volume sits between the two strip layers.
    dd4hep::Box refShape("ref_shape", 5.0, 5.0, 0.025);
    dd4hep::Volume refVol("MockMPGDRef", refShape, detector->air());
    refVol.setSensitiveDetector(sd);

    dd4hep::Box moduleShape("module_shape", 5.0, 5.0, 0.15);
    dd4hep::Box envShape("env_shape", 10.0, 10.0, 5.0);
    dd4hep::Volume envVol("MockMPGDEnvelope", envShape, detector->air());

    dd4hep::Volume worldVol     = detector->worldVolume();
    dd4hep::DetElement worldDet = detector->world();
    dd4hep::DetElement det(worldDet, "MockMPGD", 3);

    // Required for VolumeManager to accumulate parent volume IDs.
    det.object<dd4hep::DetElement::Object>().flag |=
        dd4hep::DetElement::Object::HAVE_SENSITIVE_DETECTOR;

    const int nModules = 2; // module 0 and module 1
    int deID           = 0;

    for (int imod = 0; imod < nModules; imod++) {
      dd4hep::Volume moduleVol("MockMPGDModule_" + std::to_string(imod), moduleShape,
                               detector->air());

      // Place reference volume (strip=0) at the center of the module.
      dd4hep::PlacedVolume refPV = moduleVol.placeVolume(refVol, dd4hep::Position(0, 0, 0));
      refPV.addPhysVolID("strip", 0);

      // Place p-strip (strip=1) and n-strip (strip=2) on either side.
      dd4hep::PlacedVolume stripPVs[2];
      const int stripIDs[] = {1, 2};
      for (int is = 0; is < 2; is++) {
        double zChild = (is == 0) ? -0.05 : +0.05;
        stripPVs[is]  = moduleVol.placeVolume(sensorVol, dd4hep::Position(0, 0, zChild));
        stripPVs[is].addPhysVolID("strip", stripIDs[is]);
      }

      dd4hep::PlacedVolume mpv = envVol.placeVolume(moduleVol, dd4hep::Position(0, 0, imod * 0.5));
      mpv.addPhysVolID("layer", 0);
      mpv.addPhysVolID("module", imod);

      std::string modName = "module_" + std::to_string(imod);
      dd4hep::DetElement modDE(det, modName, deID++);
      modDE.setPlacement(mpv);

      // Each sensitive placement needs its own DetElement for VolumeManager registration.
      // The reference volume (strip=0) is the "module-level" reference used by the algorithm.
      std::string refName = modName + "_strip0";
      dd4hep::DetElement refDE(modDE, refName, deID++);
      refDE.setPlacement(refPV);

      for (int is = 0; is < 2; is++) {
        std::string sensName = modName + "_strip" + std::to_string(stripIDs[is]);
        dd4hep::DetElement sensDE(modDE, sensName, deID++);
        sensDE.setPlacement(stripPVs[is]);
      }
    }

    dd4hep::PlacedVolume envPV = worldVol.placeVolume(envVol);
    envPV.addPhysVolID("system", 3);
    det.setPlacement(envPV);

    detector->endDocument();

    // NONE flag avoids auto-scanning; addSubdetector passes the correct Readout.
    dd4hep::VolumeManager vm(*detector, "tracking", detector->world(), dd4hep::Readout(),
                             dd4hep::VolumeManager::NONE);
    vm.addSubdetector(det, readoutMPGD);

    m_detector = std::move(detector);

    auto& serviceSvc              = algorithms::ServiceSvc::instance();
    [[maybe_unused]] auto& geoSvc = algorithms::GeoSvc::instance();
    serviceSvc.setInit<algorithms::GeoSvc>([this](auto&& g) { g.init(this->m_detector.get()); });

    [[maybe_unused]] auto& randomSvc = algorithms::RandomSvc::instance();
    auto seed                        = Catch::Generators::Detail::getSeed();
    serviceSvc.setInit<algorithms::RandomSvc>([seed](auto&& r) {
      r.setProperty("seed", static_cast<std::size_t>(seed));
      r.init();
    });

    auto& evaluatorSvc = eicrecon::EvaluatorSvc::instance();
    serviceSvc.add<eicrecon::EvaluatorSvc>(&evaluatorSvc);

    auto& lutSvc = eicrecon::PIDLookupTableSvc::instance();
    serviceSvc.add<eicrecon::PIDLookupTableSvc>(&lutSvc);

    auto& particleSvc = algorithms::ParticleSvc::instance();
    serviceSvc.add<algorithms::ParticleSvc>(&particleSvc);

    auto& uniqueIDSvc = algorithms::UniqueIDGenSvc::instance();
    serviceSvc.add<algorithms::UniqueIDGenSvc>(&uniqueIDSvc);

    serviceSvc.init();
  }
};

CATCH_REGISTER_LISTENER(algorithmsInitListener)
