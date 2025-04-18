
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
#include <edm4hep/Vector3f.h>
#include <DD4hep/Detector.h>
#include <DDRec/CellIDPositionConverter.h>
#include <edm4eic/Measurement2DCollection.h>

#include "extensions/spdlog/SpdlogMixin.h"

class TofEfficiency_processor : public JEventProcessorSequentialRoot, public eicrecon::SpdlogMixin {
private:
  // Containers for histograms
  std::map<std::string, TH1*> m_1d_hists;
  std::map<std::string, TH2*> m_2d_hists;

  const edm4hep::Vector3f ConvertCluster(const edm4eic::Measurement2D& cluster) const;

public:
  TofEfficiency_processor() { SetTypeName(NAME_OF_THIS); }

  void InitWithGlobalRootLock() override;
  void ProcessSequential(const std::shared_ptr<const JEvent>& event) override;
  void FinishWithGlobalRootLock() override;

  int IsTOFHit(float x, float y, float z);

  TDirectory* m_dir_main;

  TH2F* m_th2_btof_phiz;
  TH2F* m_th2_ftof_rphi;
  TNtuple* m_tntuple_track;

  const dd4hep::Detector* m_detector                      = nullptr;
  const dd4hep::rec::CellIDPositionConverter* m_converter = nullptr;
};
