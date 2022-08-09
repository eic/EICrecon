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
    


void JDD4hep_service::Initialize() {

    if( m_dd4hepGeo ) {
        LOG_WARN(default_cout_logger) << "JDD4hep_service already initialized!" << LOG_END;
    }

    m_dd4hepGeo = &(dd4hep::Detector::getInstance());

    // The current recommended way of getting the XML file is to use the environment variables
    // DETECTOR_PATH and DETECTOR. Look for those first so we can use it for the default
    // config parameter. (see https://github.com/eic/EICrecon/issues/22)
    std::vector<std::string> xml_filenames;
    auto DETECTOR = std::getenv("DETECTOR");
    auto DETECTOR_PATH = std::getenv("DETECTOR_PATH");
    if( DETECTOR!=nullptr ) xml_filenames.push_back( std::string(DETECTOR_PATH ? DETECTOR_PATH:".") + "/" + (DETECTOR ? DETECTOR:"") + ".xml");

    // User may specify multiple geometry files via the config. parameter. Normally, this
    // will be a single file which itself has include for other files.
    app->SetDefaultParameter("EIC_DD4HEP_XML", xml_filenames, "Comma separated list of XML files describing the DD4hep geometry. (Defaults to ${DETECTOR_PATH}/${DETECTOR}.xml using envars.)");

    if( xml_filenames.empty() ){
        LOG_ERROR(default_cerr_logger) << "No dd4hep XML file specified for the geometry!" << LOG_END;
        LOG_ERROR(default_cerr_logger) << "Please set the EIC_DD4HEP_XML configuration parameter or" << LOG_END;
        LOG_ERROR(default_cerr_logger) << "Set your DETECTOR_PATH and DETECTOR environment variables" << LOG_END;
        LOG_ERROR(default_cerr_logger) << "(the latter is typically done by sourcing the setup.sh" << LOG_END;
        LOG_ERROR(default_cerr_logger) << "script the epic directory.)" << LOG_END;
        throw std::runtime_error("No dd4hep XML file specified.");
    }

    // load geometry
    try {
        for (auto &filename : m_xmlFileNames) {
            std::cout << "loading geometry from file:  '" << filename << "'" << std::endl;
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
}
