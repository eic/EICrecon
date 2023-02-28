// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

// JANA
#include <JANA/JEventProcessorSequentialRoot.h>

// algorithms
#include <algorithms/digi/PhotoMultiplierHitDigiAnalysis.h>

// services
#include <services/rootfile/RootFile_service.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <extensions/string/StringHelpers.h>

namespace eicrecon {
  class Digitizer_processor :
    public JEventProcessorSequentialRoot,
    public SpdlogMixin<Digitizer_processor>
  {

    public:

      Digitizer_processor() { SetTypeName(NAME_OF_THIS); }
      void InitWithGlobalRootLock() override;
      void ProcessSequential(const std::shared_ptr<const JEvent>& event) override;
      void FinishWithGlobalRootLock() override;

    private:

      // input collections // FIXME: generalize for other RICHes
      PrefetchT<edm4eic::RawTrackerHit> m_digi_hits = {this, "DRICHRawHits"};

      // underlying algorithms
      eicrecon::PhotoMultiplierHitDigiAnalysis m_analysis_algo;

  };
}
