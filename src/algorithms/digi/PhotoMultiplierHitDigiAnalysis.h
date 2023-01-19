// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once

// ROOT
#include <TH1D.h>
#include <TH2D.h>
#include <TMath.h>

// data model
#include <edm4eic/RawPMTHitCollection.h>

// EICrecon
#include <spdlog/spdlog.h>

namespace eicrecon {

  class PhotoMultiplierHitDigiAnalysis {

    private:

      // binning
      const int adc_max = std::pow(2,10);
      const int tdc_max = std::pow(2,10);

      // histograms
      TH1D *m_adc_dist;
      TH1D *m_tdc_dist;
      TH2D *m_tdc_vs_adc;

    public:
      PhotoMultiplierHitDigiAnalysis() = default;
      ~PhotoMultiplierHitDigiAnalysis() {}

      // algorithm methods
      void AlgorithmInit(std::shared_ptr<spdlog::logger>& logger);
      void AlgorithmProcess(std::vector<const edm4eic::RawPMTHit*> hits);
      void AlgorithmFinish();

    private:
      std::shared_ptr<spdlog::logger> m_log;

  };

}
