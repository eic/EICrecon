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
      PrintError("cannot find default compact file, since env vars DETECTOR_PATH and DETECTOR_CONFIG are not set");
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
  irtDetectorCollection = new CherenkovDetectorCollection();
  irtDetector = irtDetectorCollection->AddNewDetector(detName.c_str());
}

// radiators
std::string IrtGeo::RadiatorName(int num) {
  if(num==kAerogel)  return "aerogel";
  else if(num==kGas) return "gas";
  else {
    PrintError("unknown radiator number {}",num);
    return "UNKNOWN_RADIATOR";
  }
}
const char * IrtGeo::RadiatorCStr(int num) {
  return RadiatorName(num).c_str();
}
int IrtGeo::RadiatorNum(std::string name) {
  if(name=="aerogel")  return kAerogel;
  else if(name=="gas") return kGas;
  else {
    PrintError("unknown radiator name {}",name);
    return -1;
  }
}
int IrtGeo::RadiatorNum(const char * name) {
  return RadiatorNum(std::string(name));
}
// dd4hep::DetElement *IrtGeo::RadiatorDetElement(int num) {
//   auto name = RadiatorName(num) + "_de";
//   std::transform(
//       name.begin(), name.end(), name.begin(),
//       [] (auto c) {return std::tolower(c); }
//       );
//   for(auto const& [de_name, det_elem] : detRich.children())
//     if(de_name.find(name)!=std::string::npos)
//       return &det_elem;
//   PrintError("cannot find DetElement {}",name);
//   return nullptr;
// }


IrtGeo::~IrtGeo() {
  if(irtDetector) delete irtDetector;
  if(irtDetectorCollection) delete irtDetectorCollection;
}
