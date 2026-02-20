// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Esteban Molina, Derek Anderson

#pragma once

#include <algorithms/algorithm.h>
#include <algorithms/geo.h>

#include <edm4eic/ReconstructedParticleCollection.h>

#include <string>
#include <string_view>

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/particle/ParticleConverterConfig.h"

// Class definition
namespace eicrecon {
// Define an "alias" for the templated algorithms constructor
using ParticleConverterAlgorithm =
    algorithms::Algorithm<algorithms::Input<edm4eic::ReconstructedParticleCollection>,
                          algorithms::Output<edm4eic::ReconstructedParticleCollection>>;

// Define the class of particle converter which uses as a base the algorithms class
class ParticleConverter : public ParticleConverterAlgorithm,
                          public WithPodConfig<ParticleConverterConfig> {
public:
  // Constructor of ParticleConverter inherits from the constructor of ParticleConverterAlgorithm
  ParticleConverter(std::string_view name)
      : ParticleConverterAlgorithm(name, {"inputRecoParticles"}, {"outputRecoParticles"},
                                   "Particles as such (?)") {};

  void init() final {};
  void process(const Input&, const Output&) const final;

private:
  // Services and calibrations here!
  const dd4hep::Detector* m_detector{algorithms::GeoSvc::instance().detector()};

  const dd4hep::rec::CellIDPositionConverter* m_converter{
      algorithms::GeoSvc::instance().cellIDPositionConverter()};

  std::string ecal_string = "Ecal";
  std::string hcal_string = "Hcal";
};
} // namespace eicrecon
