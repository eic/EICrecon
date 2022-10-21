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


#ifndef _PhotoMultiplierHitDigi_h_
#define _PhotoMultiplierHitDigi_h_

#include <services/geometry/dd4hep/JDD4hep_service.h>
#include <TRandomGen.h>
#include <edm4hep/SimTrackerHit.h>
#include <edm4eic/RawPMTHit.h>
#include <spdlog/spdlog.h>

class PhotoMultiplierHitDigi {

    // Insert any member variables here

public:
    PhotoMultiplierHitDigi() = default;
    ~PhotoMultiplierHitDigi(){for( auto h : rawhits ) delete h;} // better to use smart pointer?
    virtual void AlgorithmInit(std::shared_ptr<spdlog::logger>& logger);
    virtual void AlgorithmChangeRun() ;
    virtual void AlgorithmProcess() ;

    //-------- Configuration Parameters ------------
    //instantiate new spdlog logger
    std::shared_ptr<spdlog::logger> m_logger;

    std::vector<std::pair<double, double> > u_quantumEfficiency;//{this, "quantumEfficiency", {{2.6*eV, 0.3}, {7.0*eV, 0.3}}};
    double m_hitTimeWindow;//{this, "hitTimeWindow", 20.0*ns};
    double m_timeStep;//{this, "timeStep", 0.0625*ns};
    double m_speMean;//{this, "speMean", 80.0};
    double m_speError;//{this, "speError", 16.0};
    double m_pedMean;//{this, "pedMean", 200.0};
    double m_pedError;//{this, "pedError", 3.0};

    TRandomMixMax m_random;
    std::function<double()> m_rngNorm;
    std::function<double()> m_rngUni;
    //Rndm::Numbers m_rngUni, m_rngNorm;

    // inputs/outputs
    std::vector<const edm4hep::SimTrackerHit*> simhits;
    std::vector<edm4eic::RawPMTHit*> rawhits;

private:
    std::default_random_engine generator; // TODO: need something more appropriate here
    std::normal_distribution<double> m_normDist; // defaults to mean=0, sigma=1

    void qe_init();
    template<class RndmIter, typename T, class Compare> RndmIter interval_search(RndmIter beg, RndmIter end, const T &val, Compare comp) const;
    bool qe_pass(double ev, double rand) const;
};

#endif // _PhotoMultiplierHitDigi_h_
