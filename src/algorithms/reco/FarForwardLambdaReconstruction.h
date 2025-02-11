// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Sebouh Paul

#pragma once
#include <DD4hep/Detector.h>
#include <algorithms/algorithm.h>
#include <algorithms/geo.h>
#include <edm4eic/ReconstructedParticleCollection.h>
#include <spdlog/logger.h>
#include <gsl/pointers>
#include <memory>
#include <string>                                 // for basic_string
#include <string_view>                            // for string_view

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/reco/FarForwardLambdaReconstructionConfig.h"

namespace eicrecon {

using FarForwardLambdaReconstructionAlgorithm = algorithms::Algorithm<
   algorithms::Input<
      const edm4eic::ReconstructedParticleCollection,
      const edm4eic::ReconstructedParticleCollection
    >,
    algorithms::Output<
      edm4eic::ReconstructedParticleCollection,
      edm4eic::ReconstructedParticleCollection
    >
    >;
    class FarForwardLambdaReconstruction :
       public FarForwardLambdaReconstructionAlgorithm,
       public WithPodConfig<FarForwardLambdaReconstructionConfig> {
       public:
         FarForwardLambdaReconstruction(std::string_view name)
                  : FarForwardLambdaReconstructionAlgorithm{name,
                                                            {"inputNeutrons", "inputPhotons"},
                                                            {"outputLambda", "outputLambdaDecayProducts"},
                                        "Reconstructs lambda candidates and their decay products from the reconstructed neutrons and photons"} {}

         void init() final;
         void process(const Input&, const Output&) const final;

    private:
        std::shared_ptr<spdlog::logger> m_log;
        double m_neutron{0.93956542052};
      double m_pi0{0.1349768277676847};
      double m_lambda{1.115683138712051};
        const dd4hep::Detector* m_detector{algorithms::GeoSvc::instance().detector()};

    };
} // namespace eicrecon
