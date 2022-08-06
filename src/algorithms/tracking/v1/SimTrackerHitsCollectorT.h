#ifndef EICRECON_SIMTRACKERHITSCOLLECTORT_H
#define EICRECON_SIMTRACKERHITSCOLLECTORT_H

#include <type_traits>
#include <vector>
#include <edm4hep/SimTrackerHit.h>
#include <edm4hep/TrackerHit.h>


namespace eicrecon {


    template <typename OutputType>
    class SimTrackerHitsCollectorT {
    public:
        SimTrackerHitsCollectorT() {
            static_assert(std::is_base_of<edm4hep::SimTrackerHit, OutputType>::value);
        }

        std::vector<const OutputType*> process(std::vector<const edm4hep::SimTrackerHit*>);

    private:

    };

    template<typename OutputType>
    std::vector<const OutputType *>
    SimTrackerHitsCollectorT<OutputType>::process(std::vector<const edm4hep::SimTrackerHit *>) {
        return std::vector<const OutputType *>();
    }

} // eicrecon

#endif //EICRECON_SIMTRACKERHITSCOLLECTORT_H
