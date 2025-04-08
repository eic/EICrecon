// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023 Whitney Armstrong, Wouter Deconinck, David Lawrence
//

#include <JANA/JApplication.h>
#include <JANA/JException.h>
#include <JANA/Services/JServiceLocator.h>
#include <Parsers/Printout.h>
#include <TGeoManager.h>
#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>

#if __has_include(<DD4hep/plugins/DetectorChecksum.h>)
#include <DD4hep/plugins/DetectorChecksum.h>
#endif

#include "DD4hep_service.h"
#include "services/log/Log_service.h"

//----------------------------------------------------------------
// Services
//----------------------------------------------------------------
void DD4hep_service::acquire_services(JServiceLocator* srv_locator) {
  // logging service
  auto log_service = srv_locator->get<Log_service>();
  m_log            = log_service->logger("dd4hep");

  // Set the DD4hep print level to be quieter by default, but let user adjust it
  std::string print_level_str{"WARNING"};
  m_app->SetDefaultParameter("dd4hep:print_level", print_level_str,
                             "Set DD4hep print level (see DD4hep/Printout.h)");
  dd4hep::setPrintLevel(dd4hep::decodePrintLevel(print_level_str));

  // Set the TGeoManager verbose level (lower dd4hep level is more verbose)
  TGeoManager::SetVerboseLevel(dd4hep::printLevel() <= dd4hep::PrintLevel::INFO ? 1 : 0);
}

//----------------------------------------------------------------
// destructor
//----------------------------------------------------------------
DD4hep_service::~DD4hep_service() {
  try {
    if (m_dd4hepGeo) {
      m_dd4hepGeo->destroyInstance();
    }
    m_dd4hepGeo = nullptr;
  } catch (...) {
  }
}

//----------------------------------------------------------------
// detector
//
/// Return pointer to the dd4hep::Detector object.
/// Call Initialize if needed.
//----------------------------------------------------------------
gsl::not_null<const dd4hep::Detector*> DD4hep_service::detector() {
  std::call_once(init_flag, &DD4hep_service::Initialize, this);
  return m_dd4hepGeo.get();
}

//----------------------------------------------------------------
// converter
//
/// Return pointer to the cellIDPositionConverter object.
/// Call Initialize if needed.
//----------------------------------------------------------------
gsl::not_null<const dd4hep::rec::CellIDPositionConverter*> DD4hep_service::converter() {
  std::call_once(init_flag, &DD4hep_service::Initialize, this);
  return m_cellid_converter.get();
}

//----------------------------------------------------------------
// Initialize
//
/// Initialize the dd4hep geometry by reading in from the XML.
/// Note that this is called automatically the first time detector()
/// is called. Which XML file(s) are read is determined by the
/// dd4hep:xml_files configuration parameter.
//----------------------------------------------------------------
void DD4hep_service::Initialize() {

  if (m_dd4hepGeo) {
    m_log->warn("DD4hep_service already initialized!");
  }

  // The current recommended way of getting the XML file is to use the environment variables
  // DETECTOR_PATH and DETECTOR_CONFIG.
  // Look for those first, so we can use it for the default
  // config parameter.
  auto* detector_config_env = std::getenv("DETECTOR_CONFIG");
  auto* detector_path_env   = std::getenv("DETECTOR_PATH");

  std::string detector_config;
  // Check if detector_config_env is set
  if (detector_config_env != nullptr) {
    detector_config = detector_config_env;
  }

  // do we have default file name
  if (!detector_config.empty()) {
    m_xml_files.push_back(std::string(detector_path_env != nullptr ? detector_path_env : ".") +
                          "/" + detector_config + ".xml");
  }

  // User may specify multiple geometry files via the config. parameter. Normally, this
  // will be a single file which itself has includes for other files.
  m_app->SetDefaultParameter("dd4hep:xml_files", m_xml_files,
                             "Comma separated list of XML files describing the DD4hep geometry. "
                             "(Defaults to ${DETECTOR_PATH}/${DETECTOR_CONFIG}.xml using envars.)");

  if (m_xml_files.empty()) {
    m_log->error("No dd4hep XML file specified for the geometry!");
    m_log->error("Set your DETECTOR_PATH and DETECTOR_CONFIG environment variables");
    m_log->error("(the latter is typically done by sourcing the thisepic.sh");
    m_log->error("script the epic directory.)");
    throw std::runtime_error("No dd4hep XML file specified.");
  }

  // Reading the geometry may take a long time and if the JANA ticker is enabled, it will keep printing
  // while no other output is coming which makes it look like something is wrong. Disable the ticker
  // while parsing and loading the geometry
  auto tickerEnabled = m_app->IsTickerEnabled();
  m_app->SetTicker(false);

  // load geometry
  auto detector = dd4hep::Detector::make_unique("");
  try {
    m_log->info("Loading DD4hep geometry from {} files", m_xml_files.size());
    for (auto& filename : m_xml_files) {

      auto resolved_filename = resolveFileName(filename, detector_path_env);

      m_log->info("  - loading geometry file:  '{}' (patience ....)", resolved_filename);
      try {
        detector->fromCompact(resolved_filename);
      } catch (
          std::runtime_error& e) { // dd4hep throws std::runtime_error, no way to detail further
        throw JException(e.what());
      }
    }
    detector->volumeManager();
    detector->apply("DD4hepVolumeManager", 0, nullptr);
    m_cellid_converter = std::make_unique<const dd4hep::rec::CellIDPositionConverter>(*detector);

// Determine detector checksum
#if __has_include(<DD4hep/plugins/DetectorChecksum.h>)
    using dd4hep::detail::DetectorChecksum;
    DetectorChecksum checksum(*detector);
    checksum.precision    = 3;
    checksum.hash_meshes  = true;
    checksum.hash_readout = true;
    checksum.analyzeDetector(detector->world());
    DetectorChecksum::hashes_t hash_vec{checksum.handleHeader().hash};
    checksum.checksumDetElement(0, detector->world(), hash_vec, true);
    DetectorChecksum::hash_t hash =
        dd4hep::detail::hash64(&hash_vec[0], hash_vec.size() * sizeof(DetectorChecksum::hash_t));
    m_log->info("Geometry checksum 0x%016lx", hash);
#endif

    m_dd4hepGeo = std::move(detector); // const

    m_log->info("Geometry successfully loaded.");
  } catch (std::exception& e) {
    m_log->error("Problem loading geometry: {}", e.what());
    throw std::runtime_error(fmt::format("Problem loading geometry: {}", e.what()));
  }

  // Restore the ticker setting
  m_app->SetTicker(tickerEnabled);
}

std::string DD4hep_service::resolveFileName(const std::string& filename, char* detector_path_env) {

  std::string result(filename);

  // Check that this XML file actually exists.
  if (!std::filesystem::exists(result)) {

    // filename does not exist, maybe DETECTOR_PATH/filename is meant?
    if (detector_path_env != nullptr) {

      // Try looking filename in DETECTOR_PATH
      result = std::string(detector_path_env) + "/" + filename;

      if (!std::filesystem::exists(result)) {
        // Here we go against the standard practice of throwing an error and print
        // the message and exit immediately. This is because we want the last message
        // on the screen to be that this file doesn't exist.
        auto mess = fmt::format(fmt::emphasis::bold | fg(fmt::color::red), "ERROR: ");
        mess += fmt::format(fmt::emphasis::bold, "file: {} does not exist!", filename);
        mess += "\nCheck that your DETECTOR_PATH and DETECTOR_CONFIG environment variables are set "
                "correctly.";
        std::cerr << std::endl
                  << std::endl
                  << mess << std::endl
                  << std::endl; // TODO standard log here!
        std::_Exit(EXIT_FAILURE);
      }
    }
  }
  return result;
}
