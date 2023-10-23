// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023, Chao Peng, Thomas Britton, Christopher Dilks, Luigi Dello Stritto

/*  General PhotoMultiplier Digitization
 *
 *  Apply the given quantum efficiency for photon detection
 *  Converts the number of detected photons to signal amplitude
 *
 *  Author: Chao Peng (ANL)
 *  Date: 10/02/2020
 *
 *  Ported from Juggler by Thomas Britton (JLab)
 */


#pragma once

#include <DD4hep/Detector.h>
#include <DD4hep/Objects.h>
#include <DDRec/CellIDPositionConverter.h>
#include <TRandomGen.h>
#include <edm4eic/MCRecoTrackerHitAssociationCollection.h>
#include <edm4eic/RawTrackerHitCollection.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <spdlog/logger.h>
#include <stdint.h>
#include <cstddef>
#include <functional>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

#include "PhotoMultiplierHitDigiConfig.h"
#include "algorithms/interfaces/WithPodConfig.h"

namespace eicrecon {

struct PhotoMultiplierHitDigiResult {
  std::unique_ptr<edm4eic::RawTrackerHitCollection> raw_hits;
  std::unique_ptr<edm4eic::MCRecoTrackerHitAssociationCollection> hit_assocs;
};

class PhotoMultiplierHitDigi : public WithPodConfig<PhotoMultiplierHitDigiConfig> {

public:
    PhotoMultiplierHitDigi() = default;
    ~PhotoMultiplierHitDigi(){}
    void AlgorithmInit(const dd4hep::Detector* detector, const dd4hep::rec::CellIDPositionConverter* converter, std::shared_ptr<spdlog::logger>& logger);
    void AlgorithmChangeRun();
    PhotoMultiplierHitDigiResult AlgorithmProcess(
        const edm4hep::SimTrackerHitCollection* sim_hits
        );

    // EDM datatype member types
    using CellIDType = decltype(edm4hep::SimTrackerHitData::cellID);
    using TimeType   = decltype(edm4hep::SimTrackerHitData::time);

    // local structure to hold data for a hit
    struct HitData {
      uint32_t                 npe;
      double                   signal;
      TimeType                 time;
      std::vector<std::size_t> sim_hit_indices;
    };

    // random number generators
    TRandomMixMax m_random;
    std::function<double()> m_rngNorm;
    std::function<double()> m_rngUni;
    //Rndm::Numbers m_rngUni, m_rngNorm;

    // set `m_VisitAllRngPixels`, a visitor to run an action (type
    // `function<void(cellID)>`) on a selection of random CellIDs; must be
    // defined externally, since this would be detector-specific
    void SetVisitRngCellIDs(
        std::function< void(std::function<void(CellIDType)>, float) > visitor
        )
    { m_VisitRngCellIDs = visitor; }

    // set `m_PixelGapMask`, which takes `cellID` and MC hit position, returning
    // true if the hit position is on a pixel, or false if on a pixel gap; must be
    // defined externally, since this would be detector-specific
    void SetPixelGapMask(
        std::function< bool(CellIDType, dd4hep::Position) > mask
        )
    { m_PixelGapMask = mask; }

protected:

    // visitor of all possible CellIDs (set with SetVisitRngCellIDs)
    std::function< void(std::function<void(CellIDType)>, float) > m_VisitRngCellIDs =
      [] ( std::function<void(CellIDType)> visitor_action, float p ) { /* default no-op */ };

    // pixel gap mask
    std::function< bool(CellIDType, dd4hep::Position) > m_PixelGapMask =
      [] (CellIDType cellID, dd4hep::Position pos_hit_global) {
        throw std::runtime_error("pixel gap cuts enabled, but none defined");
        return false;
      };

private:

    // add a hit to local `hit_groups` data structure
    void InsertHit(
        std::unordered_map<CellIDType, std::vector<HitData>> &hit_groups,
        CellIDType       id,
        double           amp,
        TimeType         time,
        std::size_t      sim_hit_index,
        bool             is_noise_hit = false
        );

    const dd4hep::Detector* m_detector = nullptr;
    const dd4hep::rec::CellIDPositionConverter* m_converter;

    std::shared_ptr<spdlog::logger> m_log;

    // std::default_random_engine generator; // TODO: need something more appropriate here
    // std::normal_distribution<double> m_normDist; // defaults to mean=0, sigma=1

    std::vector<std::pair<double, double>> qeff;
    void qe_init();
    template<class RndmIter, typename T, class Compare> RndmIter interval_search(RndmIter beg, RndmIter end, const T &val, Compare comp) const;
    bool qe_pass(double ev, double rand) const;
};
}
