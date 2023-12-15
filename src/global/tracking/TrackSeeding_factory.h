// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <JANA/JEvent.h>
#include <edm4eic/TrackParametersCollection.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "OrthogonalTrackSeedingConfig.h"
#include "algorithms/tracking/TrackSeeding.h"
#include "extensions/jana/JOmniFactory.h"
#include "services/geometry/acts/ACTSGeo_service.h"

namespace eicrecon {

class TrackSeeding_factory :
        public JOmniFactory<TrackSeeding_factory, OrthogonalTrackSeedingConfig> {

private:
    using AlgoT = eicrecon::TrackSeeding;
    std::unique_ptr<AlgoT> m_algo;

    PodioInput<edm4eic::TrackerHit> m_hits_input {this};
    PodioOutput<edm4eic::TrackParameters> m_parameters_output {this};


    ParameterRef<float> m_rMax {this, "rMax", config().m_rMax, "max measurement radius for Acts::OrthogonalSeedFinder"};
    ParameterRef<float> m_rMin {this, "rMin", config().m_rMin, "min measurement radius for Acts::OrthogonalSeedFinder"};
    ParameterRef<float> m_deltaRMinTopSP {this, "deltaRMinTopSP", config().m_deltaRMinTopSP, "min distance in r between middle and top space point in one seed for Acts::OrthogonalSeedFinder"};
    ParameterRef<float> m_deltaRMaxTopSP {this, "deltaRMaxTopSP", config().m_deltaRMaxTopSP, "max distance in r between middle and top space point in one seed for Acts::OrthogonalSeedFinder"};
    ParameterRef<float> m_deltaRMinBottomSP {this, "deltaRMinBottomSP", config().m_deltaRMinBottomSP, "min distance in r between bottom and middle space point in one seed for Acts::OrthogonalSeedFinder"};
    ParameterRef<float> m_deltaRMaxBottomSP {this, "deltaRMaxBottomSP", config().m_deltaRMaxBottomSP, "max distance in r between bottom and middle space point in one seed for Acts::OrthogonalSeedFinder"};
    ParameterRef<float> m_collisionRegionMin {this, "collisionRegionMin", config().m_collisionRegionMin, "min location in z for collision region for Acts::OrthogonalSeedFinder"};
    ParameterRef<float> m_collisionRegionMax {this, "collisionRegionMax", config().m_collisionRegionMax, "max location in z for collision region for Acts::OrthogonalSeedFinder"};
    ParameterRef<float> m_zMax {this, "zMax", config().m_zMax, "Max z location for measurements for Acts::OrthogonalSeedFinder"};
    ParameterRef<float> m_zMin {this, "zMin", config().m_zMin, "Min z location for measurements for Acts::OrthogonalSeedFinder"};
    ParameterRef<unsigned int> m_maxSeedsPerSpM {this, "maxSeedsPerSpM", config().m_maxSeedsPerSpM, "Maximum number of seeds one space point can be the middle of for Acts::OrthogonalSeedFinder"};
    ParameterRef<float> m_cotThetaMax {this, "cotThetaMax", config().m_cotThetaMax, "cot of maximum theta angle for Acts::OrthogonalSeedFinder"};
    ParameterRef<float> m_sigmaScattering {this, "sigmaScattering", config().m_sigmaScattering, "number of sigmas of scattering angle to consider for Acts::OrthogonalSeedFinder"};
    ParameterRef<float> m_radLengthPerSeed {this, "radLengthPerSeed", config().m_radLengthPerSeed, "Approximate number of radiation lengths one seed traverses for Acts::OrthogonalSeedFinder"};
    ParameterRef<float> m_minPt {this, "minPt", config().m_minPt, "Minimum pT to search for for Acts::OrthogonalSeedFinder"};
    ParameterRef<float> m_bFieldInZ {this, "bFieldInZ", config().m_bFieldInZ, "Value of B Field to use in kiloTesla for Acts::OrthogonalSeedFinder"};
    ParameterRef<float> m_beamPosX {this, "beamPosX", config().m_beamPosX, "Beam position in x for Acts::OrthogonalSeedFinder"};
    ParameterRef<float> m_beamPosY {this, "beamPosY", config().m_beamPosY, "Beam position in y for Acts::OrthogonalSeedFinder"};
    ParameterRef<float> m_impactMax {this, "impactMax", config().m_impactMax, "maximum impact parameter allowed for seeds for Acts::OrthogonalSeedFinder. rMin should be larger than impactMax."};
    ParameterRef<float> m_rMinMiddle {this, "rMinMiddle", config().m_rMinMiddle, "min radius for middle space point for Acts::OrthogonalSeedFinder"};
    ParameterRef<float> m_rMaxMiddle {this, "rMaxMiddle", config().m_rMaxMiddle, "max radius for middle space point for Acts::OrthogonalSeedFinder"};
    ParameterRef<float> m_tolerance {this,"tolerance",config().m_tolerance,"tolerance for seed globalToLocal position conversion"};

    Service<ACTSGeo_service> m_ACTSGeoSvc {this};

public:
    void Configure() {
        m_algo = std::make_unique<AlgoT>();
        m_algo->applyConfig(config());
        m_algo->init(m_ACTSGeoSvc().actsGeoProvider(), logger());
    }

    void ChangeRun(int64_t run_number) {
    }

    void Process(int64_t run_number, uint64_t event_number) {
        m_parameters_output() = m_algo->produce(*m_hits_input());
    }
};

} // eicrecon
