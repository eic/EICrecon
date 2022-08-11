#ifndef EICRECON_SIMTRACKERHITSCOLLECTORT_H
#define EICRECON_SIMTRACKERHITSCOLLECTORT_H

#include <type_traits>
#include <vector>
#include <edm4hep/SimTrackerHit.h>
#include <edm4hep/TrackerHit.h>
#include <edm4hep/SimTrackerHitCollection.h>
#include <algorithms/interfaces/ICollectionProducer.h>

namespace eicrecon {


    template <typename OutputT>
    class SimTrackerHitsCollectorT {
    public:
        SimTrackerHitsCollectorT() {
            static_assert(std::is_base_of<edm4hep::SimTrackerHit, OutputT>::value);
        }

        std::vector<const OutputT*> process(std::vector<const edm4hep::SimTrackerHit*>);

    private:

    };

    template<typename OutputT>
    std::vector<const OutputT *> SimTrackerHitsCollectorT<OutputT>::process(std::vector<const edm4hep::SimTrackerHit *>) {
        static_assert(std::is_base_of<edm4hep::SimTrackerHit, OutputT>::value);
        auto result = std::vector<const OutputT *>();
        result[0].

    }

} // eicrecon

#endif //EICRECON_SIMTRACKERHITSCOLLECTORT_H
