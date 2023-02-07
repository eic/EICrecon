// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <cstdlib>
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <fmt/color.h>

#include "JDD4hep_service.h"

#include <DD4hep/Printout.h>

//----------------------------------------------------------------
// destructor
//----------------------------------------------------------------
JDD4hep_service::~JDD4hep_service(){
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
dd4hep::Detector* JDD4hep_service::detector() {
    std::call_once( init_flag, &JDD4hep_service::Initialize, this);
    return (m_dd4hepGeo);
}

//----------------------------------------------------------------
// Initialize
//
/// Initialize the dd4hep geometry by reading in from the XML.
/// Note that this is called automatically the first time detector()
/// is called. Which XML file(s) are read is determined by the
/// dd4hep:xml_files configuration parameter.
//----------------------------------------------------------------
void JDD4hep_service::Initialize() {

    if( m_dd4hepGeo ) {
        LOG_WARN(default_cout_logger) << "JDD4hep_service already initialized!" << LOG_END;
    }

    m_dd4hepGeo = &(dd4hep::Detector::getInstance());

    // The current recommended way of getting the XML file is to use the environment variables
    // DETECTOR_PATH and DETECTOR_CONFIG or DETECTOR(deprecated).
    // Look for those first, so we can use it for the default
    // config parameter. (see https://github.com/eic/EICrecon/issues/22)
    auto detector_config_env = std::getenv("DETECTOR_CONFIG");
    auto detector_path_env = std::getenv("DETECTOR_PATH");

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
        LOG_ERROR(default_cerr_logger) << "Please set the EIC_DD4HEP_XML configuration parameter or" << LOG_END;
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
    try {
        dd4hep::setPrintLevel(static_cast<dd4hep::PrintLevel>(print_level));
        LOG << "Loading DD4hep geometry from " << m_xml_files.size() << " files" << LOG_END;
        for (auto &filename : m_xml_files) {

            auto resolved_filename = resolveFileName(filename, detector_path_env);

            LOG << "  - loading geometry file:  '" << resolved_filename << "' (patience ....)" << LOG_END;
            try {
                m_dd4hepGeo->fromCompact(resolved_filename);
            } catch(std::runtime_error &e) {        // dd4hep throws std::runtime_error, no way to detail further
                throw JException(e.what());
            }
        }
        m_dd4hepGeo->volumeManager();
        m_dd4hepGeo->apply("DD4hepVolumeManager", 0, nullptr);
        m_cellid_converter = std::make_shared<const dd4hep::rec::CellIDPositionConverter>(*m_dd4hepGeo);

        LOG << "Geometry successfully loaded." << LOG_END;
    }catch(std::exception &e){
        LOG_ERROR(default_cerr_logger)<< "Problem loading geometry: " << e.what() << LOG_END;
        app->Quit();
    }

    // Restore the ticker setting
    app->SetTicker( tickerEnabled );
}

std::string JDD4hep_service::resolveFileName(const std::string &filename, char *detector_path_env) {

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
