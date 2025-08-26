#include "GeometryNavigationSteps_processor.h"

#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/Services/JGlobalRootLock.h>
#include <string>

#include "services/geometry/acts/ACTSGeo_service.h"
#include "services/rootfile/RootFile_service.h"

void GeometryNavigationSteps_processor::Init() {
  std::string plugin_name = ("geometry_navigation_test");

  // Get JANA application
  auto* app = GetApplication();

  // Ask service locator a file to write histograms to
  auto root_file_service = app->GetService<RootFile_service>();

  // Get TDirectory for histograms root file
  auto globalRootLock = app->GetService<JGlobalRootLock>();
  globalRootLock->acquire_write_lock();
  auto* file = root_file_service->GetHistFile();
  globalRootLock->release_lock();

  // Create a directory for this plugin. And subdirectories for series of histograms
  m_dir_main = file->mkdir(plugin_name.c_str());

  // Get log level from user parameter or default
  InitLogger(app, plugin_name);

  auto acts_service = GetApplication()->GetService<ACTSGeo_service>();
}

void GeometryNavigationSteps_processor::Process(const std::shared_ptr<const JEvent>& /* event */) {}

void GeometryNavigationSteps_processor::Finish() {}
