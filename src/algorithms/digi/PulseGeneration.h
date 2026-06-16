// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024-2025 Simon Gardner, Chun Yuen Tsang, Prithwish Tribedy,
//                         Minho Kim, Sylvester Joosten, Wouter Deconinck, Dmitry Kalinkin
//
// Convert energy deposition into analog pulses

#pragma once

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <Parsers/Primitives.h>
#include <algorithms/algorithm.h>
#include <algorithms/geo.h>
#include <edm4eic/EDM4eicVersion.h>
#include <edm4eic/unit_system.h>
#include <edm4hep/SimCalorimeterHitCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR >= 1)
#include <edm4eic/SimPulseCollection.h>
#else
#include <edm4hep/TimeSeriesCollection.h>
#endif
#include <cstddef>
#include <gsl/pointers>
#include <map>
#include <memory>
#include <random>
#include <string_view>
#include <tuple>
#include <variant>
#include <vector>

#include "algorithms/digi/PulseGenerationConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

using namespace dd4hep;

namespace eicrecon {

#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR >= 1)
using PulseType        = edm4eic::SimPulse;
using MutablePulseType = edm4eic::MutableSimPulse;
#else
using PulseType        = edm4hep::TimeSeries;
using MutablePulseType = edm4hep::MutableTimeSeries;
#endif

template <typename HitT> struct HitAdapter;

template <> struct HitAdapter<edm4hep::SimTrackerHit> {
  static std::tuple<double, double> getPulseSources(const edm4hep::SimTrackerHit& hit);
#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR >= 1)
  static void addRelations(MutablePulseType& pulse, const edm4hep::SimTrackerHit& hit);
#endif
};

template <> struct HitAdapter<edm4hep::SimCalorimeterHit> {
  static std::tuple<double, double> getPulseSources(const edm4hep::SimCalorimeterHit& hit);
#if EDM4EIC_VERSION_MAJOR > 8 || (EDM4EIC_VERSION_MAJOR == 8 && EDM4EIC_VERSION_MINOR >= 1)
  static void addRelations(MutablePulseType& pulse, const edm4hep::SimCalorimeterHit& hit);
#endif
};

template <typename HitT>
using PulseGenerationAlgorithm =
    algorithms::Algorithm<algorithms::Input<typename HitT::collection_type>,
                          algorithms::Output<PulseType::collection_type>>;

class SignalPulse;

template <typename HitT>
class PulseGeneration : public PulseGenerationAlgorithm<HitT>,
                        public WithPodConfig<PulseGenerationConfig> {

public:
  PulseGeneration(std::string_view name)
      : PulseGenerationAlgorithm<HitT>{name, {"RawHits"}, {"OutputPulses"}, {}} {}
  void init() final;
  void process(const typename PulseGenerationAlgorithm<HitT>::Input&,
               const typename PulseGenerationAlgorithm<HitT>::Output&) const final;

private:
  std::shared_ptr<SignalPulse> m_pulse;
  float m_min_sampling_time = 0 * edm4eic::unit::ns;

  // Members for converting energy deposit to the number of photoelectrons
  dd4hep::IDDescriptor id_spec;
  dd4hep::BitFieldCoder* id_dec = nullptr;
  const dd4hep::Detector* m_detector{algorithms::GeoSvc::instance().detector()};
  mutable std::mt19937 m_gen{};
  std::vector<std::size_t> m_field_idxs{};
  std::map<std::vector<int>, double> m_edep_to_npe_lut{};

private:
  double get_edep_to_npe_factor(const HitT& hit) const;
};

} // namespace eicrecon
