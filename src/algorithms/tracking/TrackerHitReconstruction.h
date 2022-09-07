// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_TRACKERHITRECONSTRUCTION_H
#define EICRECON_TRACKERHITRECONSTRUCTION_H

#include <vector>
#include <string>
#include <functional>
#include <memory>
#include <spdlog/spdlog.h>

#include <eicd/RawTrackerHit.h>
#include <eicd/TrackerHit.h>

#include <algorithms/interfaces/ICollectionProducer.h>

#include "TrackerHitReconstructionConfig.h"
#include "algorithms/interfaces/IObjectProducer.h"

#include <DD4hep/Detector.h>
#include <DDRec/CellIDPositionConverter.h>

namespace eicrecon {

    /**
     * Produces eicd::TrackerHit with geometric infor from eicd::RawTrackerHit
     *
     * IObjectProducer means that this class produces eicd::TrackerHit out of eicd::RawTrackerHit
     */
    class TrackerHitReconstruction: public IObjectProducer<eicd::RawTrackerHit, eicd::TrackerHit>{
    public:

        /// Once in a lifetime initialization
        void init(dd4hep::Detector *detector, std::shared_ptr<spdlog::logger>& logger);

        /// Processes RawTrackerHit and produces a TrackerHit
        eicd::TrackerHit* produce(const eicd::RawTrackerHit * raw_hit) override;


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



#endif //EICRECON_TRACKERHITRECONSTRUCTION_H
