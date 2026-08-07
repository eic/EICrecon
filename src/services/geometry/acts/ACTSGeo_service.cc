// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include "ACTSGeo_service.h"

#include <Acts/Visualization/ViewConfig.hpp>
#include <JANA/JApplication.h>
#include <JANA/JException.h>
#include <JANA/Services/JServiceLocator.h>
#include <array>
#include <exception>
#include <gsl/pointers>
#include <stdexcept>
#include <string>

#include "ActsGeometryProvider.h"
#include "services/geometry/dd4hep/DD4hep_service.h"
#include "services/log/Log_service.h"

// Virtual destructor implementation to pin vtable and typeinfo to this
// translation unit
ACTSGeo_service::~ACTSGeo_service() = default;

//----------------------------------------------------------------
// detector
//
/// Return pointer to the dd4hep::Detector object.
/// Call Initialize if needed.
//----------------------------------------------------------------
std::shared_ptr<const ActsGeometryProvider> ACTSGeo_service::actsGeoProvider() {

  try {
    std::call_once(m_init_flag, [this]() {
      // Assemble everything on the first call

      if (m_dd4hepGeo == nullptr) {
        throw JException("ACTSGeo_service m_dd4hepGeo==null which should never be!");
      }

      // Get material map from user parameter
      std::string material_map_file;
      try {
        material_map_file = m_dd4hepGeo->constant<std::string>("material-map");
      } catch (const std::runtime_error& e) {
        material_map_file = "calibrations/materials-map.cbor";
      }
      m_app->SetDefaultParameter("acts:MaterialMap", material_map_file,
                                 "JSON/CBOR material map file path");

      // Reading the geometry may take a long time and if the JANA ticker is enabled, it will keep printing
      // while no other output is coming which makes it look like something is wrong. Disable the ticker
      // while parsing and loading the geometry
      auto tickerEnabled = m_app->IsTickerEnabled();
      m_app->SetTicker(false);

      // Create default m_acts_provider
      m_acts_provider = std::make_shared<ActsGeometryProvider>();

      // Set ActsGeometryProvider parameters
      bool objWriteIt = m_acts_provider->getObjWriteIt();
      bool plyWriteIt = m_acts_provider->getPlyWriteIt();
      m_app->SetDefaultParameter("acts:WriteObj", objWriteIt,
                                 "Write tracking geometry as obj files");
      m_app->SetDefaultParameter("acts:WritePly", plyWriteIt,
                                 "Write tracking geometry as ply files");
      m_acts_provider->setObjWriteIt(objWriteIt);
      m_acts_provider->setPlyWriteIt(plyWriteIt);

      std::string outputTag = m_acts_provider->getOutputTag();
      std::string outputDir = m_acts_provider->getOutputDir();
      m_app->SetDefaultParameter("acts:OutputTag", outputTag, "Obj and ply output file tag");
      m_app->SetDefaultParameter("acts:OutputDir", outputDir, "Obj and ply output file dir");
      m_acts_provider->setOutputTag(outputTag);
      m_acts_provider->setOutputDir(outputDir);

      std::array<int, 3> containerView = m_acts_provider->getContainerView().color.rgb;
      std::array<int, 3> volumeView    = m_acts_provider->getVolumeView().color.rgb;
      std::array<int, 3> sensitiveView = m_acts_provider->getSensitiveView().color.rgb;
      std::array<int, 3> passiveView   = m_acts_provider->getPassiveView().color.rgb;
      std::array<int, 3> gridView      = m_acts_provider->getGridView().color.rgb;
      m_app->SetDefaultParameter("acts:ContainerView", containerView, "RGB for container views");
      m_app->SetDefaultParameter("acts:VolumeView", volumeView, "RGB for volume views");
      m_app->SetDefaultParameter("acts:SensitiveView", sensitiveView, "RGB for sensitive views");
      m_app->SetDefaultParameter("acts:PassiveView", passiveView, "RGB for passive views");
      m_app->SetDefaultParameter("acts:GridView", gridView, "RGB for grid views");
      m_acts_provider->setContainerView(containerView);
      m_acts_provider->setVolumeView(volumeView);
      m_acts_provider->setSensitiveView(sensitiveView);
      m_acts_provider->setPassiveView(passiveView);
      m_acts_provider->setGridView(gridView);

      // Initialize m_acts_provider
      m_acts_provider->initialize(m_dd4hepGeo, material_map_file, m_log, m_log);

      // Enable ticker back
      m_app->SetTicker(tickerEnabled);
    });
  } catch (std::exception& ex) {
    throw JException(ex.what());
  }

  return m_acts_provider;
}

void ACTSGeo_service::acquire_services(JServiceLocator* srv_locator) {

  auto log_service = srv_locator->get<Log_service>();
  m_log            = log_service->logger("acts");

  // DD4Hep geometry
  auto dd4hep_service = srv_locator->get<DD4hep_service>();
  m_dd4hepGeo         = dd4hep_service->detector();
}
