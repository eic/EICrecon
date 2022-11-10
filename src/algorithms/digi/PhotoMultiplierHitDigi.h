// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng

/*  General PhotoMultiplier Digitization
 *
 *  Apply the given quantum efficiency for photon detection
 *  Converts the number of detected photons to signal amplitude
 *
 *  Author: Chao Peng (ANL)
 *  Date: 10/02/2020
 */

//Ported by Thomas Britton (JLab)

#pragma once

#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <TRandomGen.h>
#include <edm4hep/SimTrackerHit.h>
#include <edm4eic/RawPMTHit.h>
#include <spdlog/spdlog.h>

#include "PhotoMultiplierHitDigiConfig.h"

namespace eicrecon {

class PhotoMultiplierHitDigi {

    // Insert any member variables here

public:
    PhotoMultiplierHitDigi() = default;
    ~PhotoMultiplierHitDigi(){for( auto h : rawhits ) delete h;} // better to use smart pointer?
    void AlgorithmInit(std::shared_ptr<spdlog::logger>& logger);
    void AlgorithmChangeRun();
    void AlgorithmProcess();

    // configurations
    eicrecon::PhotoMultiplierHitDigiConfig& getConfig() {return m_cfg;}
    eicrecon::PhotoMultiplierHitDigiConfig& applyConfig(eicrecon::PhotoMultiplierHitDigiConfig cfg) { m_cfg = cfg; return m_cfg;}

    //instantiate new spdlog logger
    std::shared_ptr<spdlog::logger> m_logger;

    TRandomMixMax m_random;
    std::function<double()> m_rngNorm;
    std::function<double()> m_rngUni;
    //Rndm::Numbers m_rngUni, m_rngNorm;

    // inputs/outputs
    std::vector<const edm4hep::SimTrackerHit*> simhits;
    std::vector<edm4eic::RawPMTHit*> rawhits;

private:
    eicrecon::PhotoMultiplierHitDigiConfig m_cfg;

    std::default_random_engine generator; // TODO: need something more appropriate here
    std::normal_distribution<double> m_normDist; // defaults to mean=0, sigma=1

    void qe_init();
    template<class RndmIter, typename T, class Compare> RndmIter interval_search(RndmIter beg, RndmIter end, const T &val, Compare comp) const;
    bool qe_pass(double ev, double rand) const;
};
}
