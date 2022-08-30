#ifndef EICRECON_SILICONTRACKERDIGI_H
#define EICRECON_SILICONTRACKERDIGI_H

#include <vector>

#include <spdlog/spdlog.h>

#include <algorithms/interfaces/ICollectionProducer.h>

#include <edm4hep/SimTrackerHit.h>
#include <eicd/RawTrackerHit.h>
#include <TRandomGen.h>

#include "SiliconTrackerDigiConfig.h"

namespace eicrecon {

    /** digitization algorithm for a silicon trackers **/
    class SiliconTrackerDigi:ICollectionProducer<edm4hep::SimTrackerHit, eicd::RawTrackerHit> {
    public:
        SiliconTrackerDigi() = default;

        /// Initialization function is called once (probably from corresponding factories)
        void init();

        /// ICollectionProducer processes RawTrackerHit collection from SimTrackerHit
        virtual std::vector<eicd::RawTrackerHit*> produce(const std::vector<const edm4hep::SimTrackerHit *>& sim_hits);

        /// Get a configuration to be changed
        eicrecon::SiliconTrackerDigiConfig& getConfig() {return m_cfg;}

        /// Sets logger to be used by the algorithm
        void setLogger(std::shared_ptr<spdlog::logger>& logger) {m_log = logger;}

    private:
        /** configuration parameters **/
        eicrecon::SiliconTrackerDigiConfig m_cfg;

        /** algorithm logger */
        std::shared_ptr<spdlog::logger> m_log;

        /** Random number generation*/
        TRandomMixMax m_random;
        std::function<double()> m_gauss;

    };

} // eicrecon

#endif //EICRECON_SILICONTRACKERDIGI_H
