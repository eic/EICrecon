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

    // Get list of XML files from environment if available to use as
    // default. Value should be comma separated list of files.
    // Normally, this will be a single file which itself has includes
    // for other files.
    auto EIC_DD4HEP_XML = std::getenv("EIC_DD4HEP_XML");
    std::string xml_filenames_str = EIC_DD4HEP_XML==nullptr ? "ecce.xml":EIC_DD4HEP_XML;
    app->SetDefaultParameter("EIC_DD4HEP_XML", xml_filenames_str, "Comma separated list of XML files describing the DD4hep geometry.");

    // Split comma separated list into single files
    std::stringstream ss(xml_filenames_str); //create string stream from the string
    while(ss.good()) {
        std::string substr;
        getline(ss, substr, ','); //get first string delimited by comma
        // trim leading and trailing spaces
        const std::string WHITESPACE = " \n\r\t\f\v";
        size_t start = substr.find_first_not_of(WHITESPACE);
        if( start == std::string::npos ) start=0;
        size_t end = substr.find_last_not_of(WHITESPACE);
        if(end == std::string::npos) end=substr.length();
        substr = substr.substr(start, end+1);
        if( ! substr.empty() ) m_xmlFileNames.push_back(substr);
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
