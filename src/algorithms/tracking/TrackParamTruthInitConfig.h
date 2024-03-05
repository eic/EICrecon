// SPDX-License-Identifier: JSA
// Copyright (C) 2022, Dmitry Romanov

#pragma once

#include <Evaluator/DD4hepUnits.h>

struct TrackParamTruthInitConfig {

    double m_maxVertexX       = 80  * dd4hep::mm;
    double m_maxVertexY       = 80  * dd4hep::mm;
    double m_maxVertexZ       = 200 * dd4hep::mm;
    double m_minMomentum      = 100 * dd4hep::MeV;
    double m_maxEtaForward    = 6.0;
    double m_maxEtaBackward   = 4.1;
    double m_momentumSmear    = 0.1;

};
