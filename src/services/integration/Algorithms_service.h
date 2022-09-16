// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_ALGORITHMS_SERVICE_H
#define EICRECON_ALGORITHMS_SERVICE_H

#include <JANA/JApplication.h>

namespace eicrecon {

class Algorithms_service: public JService {
public:
    explicit Algorithms_service( JApplication *app ): {}
    void acquire_services(JServiceLocator *) override;

private:
    Algorithms_service()=default;

};
}

#endif //EICRECON_ALGORITHMS_SERVICE_H
