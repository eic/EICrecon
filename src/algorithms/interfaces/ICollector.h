// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_ICOLLECTOR_H
#define EICRECON_ICOLLECTOR_H

namespace eicrecon {

    /** This class is just a binding intention.
     * Classes of that interface should have a method that:
     * takes many collections of one type and return one collection of that type
     * hit level conversions like:
     *
     * SimulatedHit -> ISingleProducer.produce -> DigitalizationHit
     *
     * There might be many other applications
     */
    class ICollector {

    };

} // eicrecon

#endif //EICRECON_ICOLLECTOR_H
