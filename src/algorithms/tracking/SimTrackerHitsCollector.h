//
// Created by romanov on 8/4/22.
//

#ifndef EICRECON_SIMTRACKERHITSCOLLECTOR_H
#define EICRECON_SIMTRACKERHITSCOLLECTOR_H

#include <type_traits>
#include <edm4hep/SimTrackerHit.h>
#include <edm4hep/TrackerHit.h>

namespace eicrecon {


    template <typename OutputType>
    class SimTrackerHitsCollector {
    public:
        SimTrackerHitsCollector(){
            static_assert(std::is_base_of<edm4hep::SimTrackerHit, OutputType>::value);
        }

    };

} // eicrecon

#endif //EICRECON_SIMTRACKERHITSCOLLECTOR_H
