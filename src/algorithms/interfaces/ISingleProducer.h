// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_ISINGLEPRODUCER_H
#define EICRECON_ISINGLEPRODUCER_H

namespace eicrecon {

    /** This class is just a binding intention.
     * of any that inherits this should have a method that takes one type and
     * based on whatever logic produces another class. It is very relevant for
     * hit level conversions like:
     *
     * SimulatedHit -> ISingleProducer.produce -> DigitalizationHit
     *
     * There might be many other applications
     */
    class ISingleProducer {

    };

} // eicrecon

#endif //EICRECON_ISINGLEPRODUCER_H
