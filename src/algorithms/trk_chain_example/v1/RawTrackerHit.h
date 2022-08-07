#ifndef EICRECON_TRACKERRAWHIT_H
#define EICRECON_TRACKERRAWHIT_H


#include <stdint>

namespace eicrecon {

    /**
     * This class is temporary. Juggler/Gaudi had this class in eicd
     * and there is no such thing in edm4hep
    */
    class RawTrackerHit {
    public:
        uint64_t          cellID;          // The detector specific (geometrical) cell id.
        int32_t           charge;          // ADC value
        int32_t           timeStamp;       // TDC value.

    };

} // eicrecon

#endif //EICRECON_TRACKERRAWHIT_H
