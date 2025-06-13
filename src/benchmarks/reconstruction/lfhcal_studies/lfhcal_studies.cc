#include <JANA/JApplicationFwd.h>

#include "lfhcal_studiesProcessor.h"

// The following just makes this a JANA plugin
extern "C" {
void InitPlugin(JApplication* app) {
  InitJANAPlugin(app);
  app->Add(new lfhcal_studiesProcessor());
}
}
