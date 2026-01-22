// Preprocessor command to avoid double definitions of this class during compilation
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
        using ParticleConverterAlgorithm = algorithms::Algorithm<algorithms::Input<edm4eic::ReconstructedParticleCollection>,
                                                                 algorithms::Output<edm4eic::ReconstructedParticleCollection>>;

        // Define the class of particle converter which uses as a base the algorithms class 
        class ParticleConverter : public ParticleConverterAlgorithm, public WithPodConfig<ParticleConverterConfig> {
                public:
                        // Constructor of ParticleConverter inherits from the constructor of ParticleConverterAlgorithm
                        ParticleConverter(std::string_view name) : 
                        ParticleConverterAlgorithm(name, {"inputRecoParticles"}, {"outputRecoParticles"} ,"Particles as such (?)") {}; 

                        void init() final {}; // what is the use of making it final?
                        void process(const Input&, const Output&) const final;

                private:
                        // Services and calibrations here!
                        const algorithms::GeoSvc& m_geo = algorithms::GeoSvc::instance();
        };
}