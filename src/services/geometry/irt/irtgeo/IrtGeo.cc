// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include "IrtGeo.h"

IrtGeo::IrtGeo(std::string detName_, std::string compactFile_, bool verbose_) : detName(detName_), verbose(verbose_) {

  // compact file name; if it's not been specified, try to find the default one
  std::string compactFile;
  if(compactFile_=="") {
    std::string DETECTOR_PATH(getenv("DETECTOR_PATH"));
    std::string DETECTOR_CONFIG(getenv("DETECTOR_CONFIG"));
    if(DETECTOR_PATH.empty() || DETECTOR_CONFIG.empty()) {
      PrintLog(stderr,"ERROR: cannot find default compact file, since env vars DETECTOR_PATH and DETECTOR_CONFIG are not set");
      return;
    }
    compactFile = DETECTOR_PATH + "/" + DETECTOR_CONFIG + ".xml";
  } else compactFile = compactFile_;

  // build DD4hep detector from compact file
  det = &dd4hep::Detector::getInstance();
  det->fromXML(compactFile);

  // set IRT and DD4hep geometry handles
  Bind();
}

// set IRT and DD4hep geometry handles
void IrtGeo::Bind() {
  // DD4hep geometry handles
  detRich = det->detector(detName);
  posRich = detRich.placement().position();
  // IRT geometry handles
  irtGeometry = new CherenkovDetectorCollection();
  irtDetector = irtGeometry->AddNewDetector(detName.c_str());
}

IrtGeo::~IrtGeo() {
  if(irtDetector) delete irtDetector;
  if(irtGeometry) delete irtGeometry;
}
