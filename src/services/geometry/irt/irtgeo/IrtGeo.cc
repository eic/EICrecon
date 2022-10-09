// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include "IrtGeo.h"

IrtGeo::IrtGeo(std::string detName_, std::string compactFile_) : detName(detName_) {

  // compact file name
  std::string compactFile;
  if(compactFile_=="") {
    std::string DETECTOR_PATH(getenv("DETECTOR_PATH"));
    std::string DETECTOR_CONFIG(getenv("DETECTOR_CONFIG"));
    if(DETECTOR_PATH.empty() || DETECTOR_CONFIG.empty()) {
      fmt::print(stderr,"ERROR: source environ.sh\n");
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
