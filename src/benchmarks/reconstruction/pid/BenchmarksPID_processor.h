// Copyright 2022, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

// JANA
#include <JANA/JEventProcessorSequentialRoot.h>

// data model
#include <edm4eic/CherenkovParticleID.h>

// algorithms
#include <algorithms/pid/IrtCherenkovParticleIDAnalysis.h>

// services
#include <services/rootfile/RootFile_service.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <extensions/string/StringHelpers.h>

namespace eicrecon {
  class BenchmarksPID_processor :
    public JEventProcessorSequentialRoot,
    public SpdlogMixin<BenchmarksPID_processor>
  {

    public:

      BenchmarksPID_processor() { SetTypeName(NAME_OF_THIS); }
      void InitWithGlobalRootLock() override;
      void ProcessSequential(const std::shared_ptr<const JEvent>& event) override;
      void FinishWithGlobalRootLock() override;

    private:

      // input collections
      PrefetchT<edm4eic::CherenkovParticleID> m_cherenkov_pids = {this, "DRICHIrtCherenkovParticleID"}; // FIXME: generalize for other RICHes

      // underlying algorithm
      eicrecon::IrtCherenkovParticleIDAnalysis m_analysis_algo;

  };
}
