
//
// Template for this file generated with eicmkplugin.py
//

#include <JANA/JEvent.h>
#include <JANA/JEventProcessorSequentialRoot.h>
#include <JANA/Utils/JTypeInfo.h>
#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>
#include <TNtuple.h>
#include <map>
#include <memory>
#include <string>

#include "extensions/spdlog/SpdlogMixin.h"

class TofEfficiency_processor: public JEventProcessorSequentialRoot, public eicrecon::SpdlogMixin  {
private:

    // Containers for histograms
    std::map<std::string, TH1*> m_1d_hists;
    std::map<std::string, TH2*> m_2d_hists;

public:
    TofEfficiency_processor() { SetTypeName(NAME_OF_THIS); }

    void InitWithGlobalRootLock() override;
    void ProcessSequential(const std::shared_ptr<const JEvent>& event) override;
    void FinishWithGlobalRootLock() override;

    int IsTOFHit(float x, float y, float z);

    TDirectory *m_dir_main;

    TH2F * m_th2_btof_phiz;
    TH2F * m_th2_ftof_rphi;
    TNtuple * m_tntuple_track;
};
