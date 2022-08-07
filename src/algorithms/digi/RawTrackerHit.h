#ifndef EICRECON_TRACKERRAWHIT_H
#define EICRECON_TRACKERRAWHIT_H


#include <stdint.h>

namespace eicrecon {

    /**
     * This class is temporary. Juggler/Gaudi had this class in eicd
     * and there is no such thing in edm4hep
    */
    class RawTrackerHit {
    public:
        uint64_t cellID()    { return m_cellID;}    /// The detector specific (geometrical) cell id.
        int32_t  charge()    { return m_charge;}    /// ADC value
        int32_t  timeStamp() { return m_timeStamp;} /// TDC value.

        RawTrackerHit(uint64_t cellID, int32_t charge, int32_t timeStamp) {
            m_cellID=cellID;
            m_charge=charge;
            m_timeStamp=timeStamp;
        }
    private:
        uint64_t          m_cellID;
        int32_t           m_charge;
        int32_t           m_timeStamp;



    };

} // eicrecon

#endif //EICRECON_TRACKERRAWHIT_H
