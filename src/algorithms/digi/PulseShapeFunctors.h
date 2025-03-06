// Copyright 2025, Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//
#pragma once

#include <TMath.h>

namespace eicrecon {

// ----------------------------------------------------------------------------
// Landau Pulse Shape Functor
// ----------------------------------------------------------------------------
class LandauPulse {
public:

LandauPulse(double gain, double Vm, double sigma_analog, double adc_range) : m_gain(ranges), m_sigma_analog(sigma_analog) {
    // calculation of the extreme values for Landau distribution can be found on lin 514-520 of
    // https://root.cern.ch/root/html524/src/TMath.cxx.html#fsokrB Landau reaches minimum for mpv =
    // 0 and sigma = 1 at x = -0.22278
    const double x_when_landau_min = -0.22278;
    const double landau_min    = -gain * TMath::Landau(x_when_landau_min, 0, 1, kTRUE) / m_sigma_analog;
    m_scalingFactor = 1. / Vm / landau_min * adc_range;
};

double operator()(double time) const {
    return m_charge * m_gain * TMath::Landau(time, m_hit_time, m_sigma_analog, kTRUE) * m_scalingFactor;
}

void SetCharge(double charge) {
    m_charge = charge;
}

void SetHitTime(double hit_time) {
    m_hit_time = hit_time;
}

private:
    double m_charge;
    double m_hit_time;
    const double m_gain;
    const double m_sigma_analog;
    double m_scalingFactor;
};

} // eicrecon
