// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <vector>
#include <string>
#include <functional>
#include <memory>
#include <spdlog/spdlog.h>

#include <edm4eic/RawTrackerHit.h>
#include <edm4eic/TrackerHit.h>

#include <algorithms/interfaces/ICollectionProducer.h>

#include "TrackerHitReconstructionConfig.h"
#include "algorithms/interfaces/IObjectProducer.h"

#include <DD4hep/Detector.h>
#include <DDRec/CellIDPositionConverter.h>

namespace eicrecon {

    /**
     * Produces edm4eic::TrackerHit with geometric infor from edm4eic::RawTrackerHit
     *
     * IObjectProducer means that this class produces edm4eic::TrackerHit out of edm4eic::RawTrackerHit
     */
    class TrackerHitReconstruction: public IObjectProducer<edm4eic::RawTrackerHit, edm4eic::TrackerHit>{
    public:

        /// Once in a lifetime initialization
        void init(dd4hep::Detector *detector, std::shared_ptr<spdlog::logger>& logger);

        /// Processes RawTrackerHit and produces a TrackerHit
        edm4eic::TrackerHit* produce(const edm4eic::RawTrackerHit * raw_hit) override;


        /// Get a configuration to be changed
        eicrecon::TrackerHitReconstructionConfig& getConfig() {return m_cfg;}

    private:
        /** configuration parameters **/
        eicrecon::TrackerHitReconstructionConfig m_cfg;

        /** algorithm logger */
        std::shared_ptr<spdlog::logger> m_log;

        /// Cell ID position converter
        std::shared_ptr<const dd4hep::rec::CellIDPositionConverter> m_cellid_converter;
    };
}
