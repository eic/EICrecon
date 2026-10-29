// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Chun Yuen Tsang, Minho Kim, Aiden Wu

#pragma once

#include <DD4hep/DetElement.h>
#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DDRec/CellIDPositionConverter.h>
#include <Parsers/Primitives.h>
#include <algorithms/algorithm.h>
#include <algorithms/geo.h>
#include <edm4eic/CalorimeterHitCollection.h>
#include <edm4eic/MCRecoCalorimeterHitAssociationCollection.h>
#include <edm4eic/MCRecoCalorimeterHitLinkCollection.h>
#include <edm4eic/RawCALOROCHitCollection.h>
#include <edm4eic/SimPulseCollection.h>
#include <edm4hep/RawCalorimeterHitCollection.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <tuple>
#include <gsl/pointers>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "CalorimeterCALOROCRecoConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

using CalorimeterCALOROCRecoAlgorithm = algorithms::Algorithm<
    algorithms::Input<edm4hep::SimCalorimeterHitCollection, edm4eic::RawCALOROCHitCollection>,
    algorithms::Output<edm4eic::CalorimeterHitCollection, edm4hep::RawCalorimeterHitCollection>>;

class CalorimeterCALOROCReco : public CalorimeterCALOROCRecoAlgorithm,
                               public WithPodConfig<CalorimeterCALOROCRecoConfig> {

public:
  CalorimeterCALOROCReco(std::string_view name)
      : CalorimeterCALOROCRecoAlgorithm{
            name,
            {"inputNpeHitCollection", "inputADCCollection"},
            {"outputRecHitCollection", "outputRawHitCollection"},
            "Reconstruct single-sided hits from CALOROC ADC data (energy and time calibration)."} {}

  // Pulses follow raw-hit order; source subset indices follow converted SiPM hits.
  using ADCInput = std::tuple<const edm4eic::RawCALOROCHitCollection*,
                              const edm4eic::SimPulseCollection*,
                              const edm4hep::SimCalorimeterHitCollection*>;
  using ADCOutput = std::tuple<edm4eic::CalorimeterHitCollection*,
                               edm4hep::RawCalorimeterHitCollection*,
                               edm4eic::MCRecoCalorimeterHitLinkCollection*,
                               edm4eic::MCRecoCalorimeterHitAssociationCollection*>;

  void init() final;
  void process(const Input&, const Output&) const final;
  void process(const ADCInput&, const ADCOutput&) const;

private:
  struct SignalMeasurements {
    double adc_sum{0};
    double tot{0};
    double time{0};
    bool adc_saturated{false};
    bool found_time{false};
  };

  struct CALOROC1BParameters {
    unsigned int high_gain_limit;
    double gain_ratio;
    double charge_scale;
    double pedestal;
  };

  struct HitGeometry {
    decltype(edm4eic::CalorimeterHitData::position) position;
    decltype(edm4eic::CalorimeterHitData::dimension) dimension;
    decltype(edm4eic::CalorimeterHitData::local) local;
    int layer{-1};
    int sector{-1};
  };

  std::optional<double> _calibrateEnergyADC(std::uint64_t cellID, const SignalMeasurements& samples) const;
  std::optional<double> _calibrateEnergyLUT(std::uint64_t cellID, double charge) const;
  double _timeWalkCorrection(const SignalMeasurements& samples) const;
  std::optional<HitGeometry> _getHitGeometry(std::uint64_t cellID) const;
  edm4hep::MutableRawCalorimeterHit _createHits(std::uint64_t cellID, double charge, double energy,
                                              double time, const HitGeometry& geometry,
                                              const Output& output) const;
  void _createTruthLinks(const edm4hep::RawCalorimeterHit& raw_hit,
                         const edm4eic::SimPulse& pulse,
                         const edm4hep::SimCalorimeterHitCollection& source_hits,
                         const ADCOutput& output) const;

  std::function<double(std::uint64_t)> sampFrac;
  dd4hep::IDDescriptor id_spec;
  dd4hep::BitFieldCoder* id_dec = nullptr;

  std::size_t sector_idx{0}, layer_idx{0};

  mutable bool warned_unsupported_segmentation = false;

  dd4hep::DetElement m_local;
  std::size_t local_mask = ~static_cast<std::size_t>(0), gpos_mask = static_cast<std::size_t>(0);

  std::map<std::vector<int>, double> m_edep_to_npe_lut{};
  std::vector<std::size_t> m_field_idxs{};

  SignalMeasurements _reconstruct1A(const edm4eic::RawCALOROCHit& hit) const;
  SignalMeasurements _reconstruct1B(const edm4eic::RawCALOROCHit& ADC) const;
  CALOROC1BParameters _getLUT1BParameters() const;
  double _toa(const edm4eic::RawCALOROCHit& hit, std::size_t arrival_index, unsigned int toa) const;

private:
  const dd4hep::Detector* m_detector{algorithms::GeoSvc::instance().detector()};
  const dd4hep::rec::CellIDPositionConverter* m_converter{
      algorithms::GeoSvc::instance().cellIDPositionConverter()};
};

} // namespace eicrecon
