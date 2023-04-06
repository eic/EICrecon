// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

// JANA
#include <JANA/JEventProcessorSequentialRoot.h>

// algorithms
#include <algorithms/pid/LinkParticleIDAnalysis.h>

// services
#include <services/rootfile/RootFile_service.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <extensions/string/StringHelpers.h>

namespace eicrecon {
  class LinkParticleID_processor :
    public JEventProcessorSequentialRoot,
    public SpdlogMixin<LinkParticleID_processor>
  {

    public:

      LinkParticleID_processor() { SetTypeName(NAME_OF_THIS); }
      void InitWithGlobalRootLock() override;
      void ProcessSequential(const std::shared_ptr<const JEvent>& event) override;
      void FinishWithGlobalRootLock() override;

    private:

      // input collections
      PrefetchT<edm4eic::MCRecoParticleAssociation> m_assocs = {this, "ReconstructedChargedParticlesAssociations"};

      // underlying algorithms
      eicrecon::LinkParticleIDAnalysis m_analysis_algo;

      // other objects
      TDirectory *m_rootdir;
  };
}
