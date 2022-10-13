// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_TRACKPARAMTRUTHINITCONFIG_H
#define EICRECON_TRACKPARAMTRUTHINITCONFIG_H

#include <Acts/Definitions/Units.hpp>

struct TrackParamTruthInitConfig {

    double m_maxVertexX       = 80  * Acts::UnitConstants::mm;
    double m_maxVertexY       = 80  * Acts::UnitConstants::mm;
    double m_maxVertexZ       = 200 * Acts::UnitConstants::mm;
    double m_minMomentum      = 100 * Acts::UnitConstants::MeV;
    double m_maxEtaForward    = 4.0;
    double m_maxEtaBackward   = 4.1;
    double phi_smearing = 0;            // phi + phi_smearing*gRandom->Gaus(0.,phi);
    double theta_smearing = 0;          // theta + theta_smearing*gRandom->Gaus(0.,theta);
    double pmag_smearing = 0;           // pmag_true + pmag_smearing*gRandom->Gaus(0.,pmag_true);

};


#endif //EICRECON_TRACKPARAMTRUTHINITCONFIG_H
