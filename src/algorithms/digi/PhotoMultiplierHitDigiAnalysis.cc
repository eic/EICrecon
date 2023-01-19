// Copyright 2023, Christopher Dilks
// Subject to the terms in the LICENSE file found in the top-level directory.

#include "PhotoMultiplierHitDigiAnalysis.h"

// AlgorithmInit
//---------------------------------------------------------------------------
void eicrecon::PhotoMultiplierHitDigiAnalysis::AlgorithmInit(std::shared_ptr<spdlog::logger>& logger) {
  m_log = logger;

  // define histograms
  m_adc_dist = new TH1D("adc_dist", "ADC Distribution;counts", adc_max, 0, adc_max);
  m_tdc_dist = new TH1D("tdc_dist", "TDC Distribution;counts", tdc_max, 0, tdc_max);
  m_tdc_vs_adc = new TH2D("tdc_vs_adc", "TDC vs. ADC;ADC counts;TDC counts",
      adc_max/2, 0, adc_max/2,
      tdc_max/2, 0, tdc_max/2
      );

  // format histograms
  auto format1D = [] (auto h) {
    h->SetLineColor(kBlack);
    h->SetFillColor(kBlack);
  };
  format1D(m_adc_dist);
  format1D(m_tdc_dist);
}


// AlgorithmProcess
//---------------------------------------------------------------------------
void eicrecon::PhotoMultiplierHitDigiAnalysis::AlgorithmProcess(std::vector<const edm4eic::RawPMTHit*> hits) {
  m_log->trace("{:=^70}"," call PhotoMultiplierHitDigiAnalysis::AlgorithmProcess ");

  // loop over `CherenkovParticleID` objects
  for(const auto& hit : hits) {
    auto adc = hit->getIntegral();
    auto tdc = hit->getTimeStamp();
    m_adc_dist->Fill(adc);
    m_tdc_dist->Fill(tdc);
    m_tdc_vs_adc->Fill(adc,tdc);
  }
}


// AlgorithmFinish
//---------------------------------------------------------------------------
void eicrecon::PhotoMultiplierHitDigiAnalysis::AlgorithmFinish() {
}
