// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

// ROOT
#include <TH2D.h>
#include <TGraphErrors.h>

// data model
#include <edm4eic/MCRecoParticleAssociationCollection.h>
#include <edm4hep/utils/kinematics.h>

// EICrecon
#include <spdlog/spdlog.h>
#include "Tools.h"

namespace eicrecon {

  class LinkParticleIDAnalysis {

    public:
      LinkParticleIDAnalysis() = default;
      ~LinkParticleIDAnalysis() {}

      // algorithm methods
      void AlgorithmInit(std::shared_ptr<spdlog::logger>& logger);
      void AlgorithmProcess(
          std::vector<const edm4eic::MCRecoParticleAssociation*> in_assocs
          );
      void AlgorithmFinish();

    private:

      // binning
      static constexpr int momentum_bins = 500;
      static constexpr int momentum_max  = 70;

      // histograms
      TGraphErrors *m_correct_vs_p;
      TH2D         *m_correct_vs_p_transient; // (not written)

      // additional objects
      std::shared_ptr<spdlog::logger> m_log;

  };

}
