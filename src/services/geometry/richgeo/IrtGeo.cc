// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include "IrtGeo.h"

// constructor: creates IRT-DD4hep bindings using main `Detector` handle `*det_`
richgeo::IrtGeo::IrtGeo(std::string detName_, dd4hep::Detector *det_, bool verbose_) :
  m_detName(detName_), m_det(det_), m_log(Logger::Instance(verbose_))
{
  Bind();
}

// alternate constructor: use compact file for DD4hep geometry (backward compatibility)
richgeo::IrtGeo::IrtGeo(std::string detName_, std::string compactFile_, bool verbose_) :
  m_detName(detName_), m_log(Logger::Instance(verbose_))
{
  // compact file name; if it's not been specified, try to find the default one
  std::string compactFile;
  if(compactFile_=="") {
    std::string DETECTOR_PATH(getenv("DETECTOR_PATH"));
    std::string DETECTOR_CONFIG(getenv("DETECTOR_CONFIG"));
    if(DETECTOR_PATH.empty() || DETECTOR_CONFIG.empty()) {
      m_log.PrintError("cannot find default compact file, since env vars DETECTOR_PATH and DETECTOR_CONFIG are not set");
      return;
    }
    compactFile = DETECTOR_PATH + "/" + DETECTOR_CONFIG + ".xml";
  } else compactFile = compactFile_;

  // build DD4hep detector from compact file
  m_det = &dd4hep::Detector::getInstance();
  m_det->fromXML(compactFile);

  // set IRT and DD4hep geometry handles
  Bind();
}

// set IRT and DD4hep geometry handles
void richgeo::IrtGeo::Bind() {
  // DD4hep geometry handles
  m_detRich = m_det->detector(m_detName);
  m_posRich = m_detRich.placement().position();
  // IRT geometry handles
  m_irtDetectorCollection = new CherenkovDetectorCollection();
  m_irtDetector = m_irtDetectorCollection->AddNewDetector(m_detName.c_str());
}

richgeo::IrtGeo::~IrtGeo() {
  if(m_irtDetector) delete m_irtDetector;
  if(m_irtDetectorCollection) delete m_irtDetectorCollection;
}
