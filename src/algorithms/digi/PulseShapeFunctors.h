// Copyright 2025, Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//
#pragma once

#include <TMath.h>

namespace eicrecon {

class SignalPulse {

public:
    virtual double operator()(double time) const = 0;

    void setHitCharge(double charge) {
        m_charge = charge;
    }

    void setHitTime(double hit_time) {
        m_hit_time = hit_time;
    }

    virtual float getMaximumTime() const = 0;

protected:
    double m_charge;
    double m_hit_time;

};

// ----------------------------------------------------------------------------
// Landau Pulse Shape Functor
// ----------------------------------------------------------------------------
class LandauPulse: public SignalPulse {
public:

LandauPulse(double gain, double sigma_analog) : m_gain(gain), m_sigma_analog(sigma_analog) {};

double operator()(double time) const {
    return m_charge * m_gain * TMath::Landau(time, m_hit_time+3.5*m_sigma_analog, m_sigma_analog, kTRUE);
}

float getMaximumTime() const {
    return m_hit_time+3.5*m_sigma_analog;
}

private:
    const double m_gain;
    const double m_sigma_analog;
};

} // eicrecon
