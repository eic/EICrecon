// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023 Whitney Armstrong, Wouter Deconinck, David Lawrence
//

#include <JANA/JException.h>
#include <JANA/JLogger.h>
#include <Parsers/Printout.h>
#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <algorithm>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>

#include "DD4hep_service.h"

//----------------------------------------------------------------
// destructor
//----------------------------------------------------------------
DD4hep_service::~DD4hep_service(){
    try {
        if(m_dd4hepGeo) m_dd4hepGeo->destroyInstance();
        m_dd4hepGeo = nullptr;
    } catch (...) {}
}

//----------------------------------------------------------------
// detector
//
/// Return pointer to the dd4hep::Detector object.
/// Call Initialize if needed.
//----------------------------------------------------------------
gsl::not_null<const dd4hep::Detector*>
DD4hep_service::detector() {
    std::call_once(init_flag, &DD4hep_service::Initialize, this);
    return m_dd4hepGeo.get();
}

//----------------------------------------------------------------
// converter
//
/// Return pointer to the cellIDPositionConverter object.
/// Call Initialize if needed.
//----------------------------------------------------------------
gsl::not_null<const dd4hep::rec::CellIDPositionConverter*>
DD4hep_service::converter() {
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
        LOG_WARN(default_cout_logger) << "DD4hep_service already initialized!" << LOG_END;
    }

    // The current recommended way of getting the XML file is to use the environment variables
    // DETECTOR_PATH and DETECTOR_CONFIG or DETECTOR(deprecated).
    // Look for those first, so we can use it for the default
    // config parameter. (see https://github.com/eic/EICrecon/issues/22)
    auto *detector_config_env = std::getenv("DETECTOR_CONFIG");
    auto *detector_path_env = std::getenv("DETECTOR_PATH");

    std::string detector_config;
    // Check if detector_config_env is set
    if(detector_config_env != nullptr) {
        detector_config = detector_config_env;
    }

    // do we have default file name
    if(!detector_config.empty()) {
        m_xml_files.push_back(std::string(detector_path_env ? detector_path_env : ".") + "/" + detector_config + ".xml");
    }

    // User may specify multiple geometry files via the config. parameter. Normally, this
    // will be a single file which itself has includes for other files.
    app->SetDefaultParameter("dd4hep:xml_files", m_xml_files, "Comma separated list of XML files describing the DD4hep geometry. (Defaults to ${DETECTOR_PATH}/${DETECTOR_CONFIG}.xml using envars.)");

    if( m_xml_files.empty() ){
        LOG_ERROR(default_cerr_logger) << "No dd4hep XML file specified for the geometry!" << LOG_END;
        LOG_ERROR(default_cerr_logger) << "Set your DETECTOR_PATH and DETECTOR_CONFIG environment variables" << LOG_END;
        LOG_ERROR(default_cerr_logger) << "(the latter is typically done by sourcing the setup.sh" << LOG_END;
        LOG_ERROR(default_cerr_logger) << "script the epic directory.)" << LOG_END;
        throw std::runtime_error("No dd4hep XML file specified.");
    }

    // Set the DD4hep print level to be quieter by default, but let user adjust it
    int print_level = dd4hep::WARNING;
    app->SetDefaultParameter("dd4hep:print_level", print_level, "Set DD4hep print level (see DD4hep/Printout.h)");

    // Reading the geometry may take a long time and if the JANA ticker is enabled, it will keep printing
    // while no other output is coming which makes it look like something is wrong. Disable the ticker
    // while parsing and loading the geometry
    auto tickerEnabled = app->IsTickerEnabled();
    app->SetTicker( false );

    // load geometry
    auto detector = dd4hep::Detector::make_unique("");
    try {
        dd4hep::setPrintLevel(static_cast<dd4hep::PrintLevel>(print_level));
        LOG << "Loading DD4hep geometry from " << m_xml_files.size() << " files" << LOG_END;
        for (auto &filename : m_xml_files) {

            auto resolved_filename = resolveFileName(filename, detector_path_env);

            LOG << "  - loading geometry file:  '" << resolved_filename << "' (patience ....)" << LOG_END;
            try {
                detector->fromCompact(resolved_filename);
            } catch(std::runtime_error &e) {        // dd4hep throws std::runtime_error, no way to detail further
                throw JException(e.what());
            }
        }
        detector->volumeManager();
        detector->apply("DD4hepVolumeManager", 0, nullptr);
        m_cellid_converter = std::make_unique<const dd4hep::rec::CellIDPositionConverter>(*detector);
        m_dd4hepGeo = std::move(detector); // const

        LOG << "Geometry successfully loaded." << LOG_END;
    } catch(std::exception &e) {
        LOG_ERROR(default_cerr_logger)<< "Problem loading geometry: " << e.what() << LOG_END;
        throw std::runtime_error(fmt::format("Problem loading geometry: {}", e.what()));
    }

    // Restore the ticker setting
    app->SetTicker( tickerEnabled );
}

std::string DD4hep_service::resolveFileName(const std::string &filename, char *detector_path_env) {

    std::string result(filename);

    // Check that this XML file actually exists.
    if( ! std::filesystem::exists(result) ){

        // filename does not exist, maybe DETECTOR_PATH/filename is meant?
        if(detector_path_env) {

            // Try looking filename in DETECTOR_PATH
            result = std::string(detector_path_env) + "/" + filename;

            if( ! std::filesystem::exists(result) ) {
                // Here we go against the standard practice of throwing an error and print
                // the message and exit immediately. This is because we want the last message
                // on the screen to be that this file doesn't exist.
                auto mess = fmt::format(fmt::emphasis::bold | fg(fmt::color::red), "ERROR: ");
                mess += fmt::format(fmt::emphasis::bold, "file: {} does not exist!", filename);
                mess += "\nCheck that your DETECTOR and DETECTOR_CONFIG environment variables are set correctly.";
                std::cerr << std::endl << std::endl << mess << std::endl << std::endl; // TODO standard log here!
                std::_Exit(EXIT_FAILURE);
            }
        }
    }
    return result;
}
