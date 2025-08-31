

#include <JANA/JEventSourceGenerator.h>
#include "JEventSourcePODIO.h"

class JEventSourcePODIO_generator : public JEventSourceGenerator {

  JEventSource* MakeJEventSource(std::string resource_name) override {

    auto* source = new JEventSourcePODIO();
    source->SetTypeName("JEventSourcePODIO");
    source->SetResourceName(resource_name);
    source->SetApplication(mApplication);
    source->SetPluginName(GetPluginName());

    // Check if the string "timeslices" appears anywhere in our filename.
    // If so, we assume the file contains timeslices, otherwise it contains physics events.
    // Another approach might be to peek at the file's contents
    /*
    if (resource_name.find("timeframe") != std::string::npos) {
      source->SetLevel(JEventLevel::Timeslice);
    } else {
      source->SetLevel(JEventLevel::PhysicsEvent);
    }
    */

    // TODO: NWB: Right now we are ALWAYS running the splitter, even when not necessary
    //            This is so we can run CI tests against the entire reco pipeline
    //            under the assumption timeframe = physicsevent

    source->SetLevel(JEventLevel::Timeslice);
    return source;
  }

  double CheckOpenable(std::string resource_name) override {
    // In theory, we should check whether PODIO can open the file and
    // whether it contains an 'events' or 'timeslices' tree. If not, return 0.
    if (resource_name.find(".root") != std::string::npos) {
      return 0.01;
    }
    return 0;
  }

  std::string GetType() const override { return "JEventSourcePodio"; }
};
