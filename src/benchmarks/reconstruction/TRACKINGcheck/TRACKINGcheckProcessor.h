
//
// Template for this file generated with eicmkplugin.py
//

#include <JANA/JEventProcessorSequentialRoot.h>
#include <JANA/Utils/JTypeInfo.h>
#include <map>
#include <memory>
#include <string>

class JEvent;
class TH1;
class TH2;

class TRACKINGcheckProcessor : public JEventProcessorSequentialRoot {
private:
  // Containers for histograms
  std::map<std::string, TH1*> hist1D;
  std::map<std::string, TH2*> hist2D;

public:
  TRACKINGcheckProcessor() { SetTypeName(NAME_OF_THIS); }

  void InitWithGlobalRootLock() override;
  void ProcessSequential(const std::shared_ptr<const JEvent>& event) override;
  void FinishWithGlobalRootLock() override;
};
