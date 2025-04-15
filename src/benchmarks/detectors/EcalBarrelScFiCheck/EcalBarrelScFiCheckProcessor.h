
//
// Template for this file generated with eicmkplugin.py
//

#include <JANA/JEvent.h>
#include <JANA/JEventProcessorSequentialRoot.h>
#include <JANA/Utils/JTypeInfo.h>
#include <TH1.h>
#include <TH2.h>
#include <map>
#include <memory>
#include <string>

class EcalBarrelScFiCheckProcessor : public JEventProcessorSequentialRoot {
private:
  // Declare histogram and tree pointers
  std::map<std::string, TH1*> hist1D;
  std::map<std::string, TH2*> hist2D;

public:
  EcalBarrelScFiCheckProcessor() { SetTypeName(NAME_OF_THIS); }

  void InitWithGlobalRootLock() override;
  void ProcessSequential(const std::shared_ptr<const JEvent>& event) override;
  void FinishWithGlobalRootLock() override;
};
