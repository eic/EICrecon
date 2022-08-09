// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <cstdlib>
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>

#include "JDD4hep_service.h"
    
#include <DD4hep/Printout.h>

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
    // DETECTOR_PATH and DETECTOR. Look for those first so we can use it for the default
    // config parameter. (see https://github.com/eic/EICrecon/issues/22)
    auto DETECTOR = std::getenv("DETECTOR");
    auto DETECTOR_PATH = std::getenv("DETECTOR_PATH");
    if( DETECTOR!=nullptr ) m_xmlFileNames.push_back( std::string(DETECTOR_PATH ? DETECTOR_PATH:".") + "/" + (DETECTOR ? DETECTOR:"") + ".xml");

    // User may specify multiple geometry files via the config. parameter. Normally, this
    // will be a single file which itself has include for other files.
    app->SetDefaultParameter("dd4hep:xml_files", m_xmlFileNames, "Comma separated list of XML files describing the DD4hep geometry. (Defaults to ${DETECTOR_PATH}/${DETECTOR}.xml using envars.)");

    if( m_xmlFileNames.empty() ){
        LOG_ERROR(default_cerr_logger) << "No dd4hep XML file specified for the geometry!" << LOG_END;
        LOG_ERROR(default_cerr_logger) << "Please set the EIC_DD4HEP_XML configuration parameter or" << LOG_END;
        LOG_ERROR(default_cerr_logger) << "Set your DETECTOR_PATH and DETECTOR environment variables" << LOG_END;
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
        LOG << "Loading DD4hep geometry from " << m_xmlFileNames.size() << " files" << LOG_END;
        for (auto &filename : m_xmlFileNames) {
            LOG << "  - loading geometry file:  '" << filename << "' (patience ....)" << LOG_END;
            m_dd4hepGeo->fromCompact(filename);
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
