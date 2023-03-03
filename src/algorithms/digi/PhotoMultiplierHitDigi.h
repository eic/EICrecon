// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Chao Peng, Christopher Dilks

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
#include <edm4hep/SimTrackerHitCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <spdlog/spdlog.h>
#include <Evaluator/DD4hepUnits.h>

#include "PhotoMultiplierHitDigiConfig.h"
#include <algorithms/interfaces/WithPodConfig.h>

namespace eicrecon {

struct PhotoMultiplierHitDigiResult {
  std::vector<edm4eic::RawTrackerHit*> raw_hits;
  std::vector<edm4eic::MCRecoTrackerHitAssociation> hit_associations;
}

class PhotoMultiplierHitDigi : public WithPodConfig<PhotoMultiplierHitDigiConfig> {

public:
    PhotoMultiplierHitDigi() = default;
    ~PhotoMultiplierHitDigi(){}
    void AlgorithmInit(dd4hep::Detector *detector, std::shared_ptr<spdlog::logger>& logger);
    void AlgorithmChangeRun();
    PhotoMultiplierHitDigiResult AlgorithmProcess(std::vector<const edm4hep::SimTrackerHit*>& sim_hits);

    // transform global position `pos` to sensor `id` frame position
    // IMPORTANT NOTE: this has only been tested for the dRICH; if you use it, test it carefully...
    dd4hep::Position get_sensor_local_position(uint64_t id, dd4hep::Position pos);

    // random number generators
    TRandomMixMax m_random;
    std::function<double()> m_rngNorm;
    std::function<double()> m_rngUni;
    //Rndm::Numbers m_rngUni, m_rngNorm;

    // convert dd4hep::Position <-> edm4hep::Vector3d
    edm4hep::Vector3d pos2vec(dd4hep::Position p) {
      return edm4hep::Vector3d( p.x()/dd4hep::mm, p.y()/dd4hep::mm, p.z()/dd4hep::mm );
    }
    dd4hep::Position vec2pos(edm4hep::Vector3d v) {
      return dd4hep::Position( v.x*dd4hep::mm, v.y*dd4hep::mm, v.z*dd4hep::mm );
    }


private:

    std::shared_ptr<spdlog::logger> m_log;
    std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> m_cellid_converter;

    // std::default_random_engine generator; // TODO: need something more appropriate here
    // std::normal_distribution<double> m_normDist; // defaults to mean=0, sigma=1

    std::vector<std::pair<double, double>> qeff;
    void qe_init();
    template<class RndmIter, typename T, class Compare> RndmIter interval_search(RndmIter beg, RndmIter end, const T &val, Compare comp) const;
    bool qe_pass(double ev, double rand) const;
};
}
