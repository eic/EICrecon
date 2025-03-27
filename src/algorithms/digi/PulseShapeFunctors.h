// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Simon Gardner
#pragma once

#include <TMath.h>

namespace eicrecon {

class SignalPulse {

public:
    virtual double operator()(double time) const = 0;

    void setHitCharge(double charge) {
        m_charge = charge;
    }

protected:
    double m_charge;

};

// ----------------------------------------------------------------------------
// Landau Pulse Shape Functor
// ----------------------------------------------------------------------------
class LandauPulse: public SignalPulse {
public:

LandauPulse(double gain, double sigma_analog) : m_gain(gain), m_sigma_analog(sigma_analog) {};

double operator()(double time) const {
    return m_charge * m_gain * TMath::Landau(time, 3.5*m_sigma_analog, m_sigma_analog, kTRUE);
}

private:
    const double m_gain;
    const double m_sigma_analog;
};

} // eicrecon
