// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Simon Gardner
#pragma once

#include <TMath.h>
#include "services/evaluator/EvaluatorSvc.h" 

namespace eicrecon {



class SignalPulse {

public:
    virtual ~SignalPulse() = default; // Virtual destructor

    virtual double operator()(double time, double charge) = 0;
    
    virtual float getMaximumTime() const = 0;

protected:
    double m_charge;

};

// ----------------------------------------------------------------------------
// Landau Pulse Shape Functor
// ----------------------------------------------------------------------------
class LandauPulse: public SignalPulse {
public:

LandauPulse(std::vector<double> params) {

    if (params.size() != 2) {
        throw std::runtime_error("LandauPulse requires 2 parameters");
    }

    m_gain = params[0];
    m_sigma_analog = params[1];

};

double operator()(double time, double charge) {
    return charge * m_gain * TMath::Landau(time, 3.5*m_sigma_analog, m_sigma_analog, kTRUE);
}

float getMaximumTime() const {
    return 3.5*m_sigma_analog;
}

private:
    double m_gain;
    double m_sigma_analog;
};

// EvaluatorSvc Pulse
class EvaluatorPulse: public SignalPulse {
public:

    EvaluatorPulse(const std::string& expression, const std::vector<double>& params) {

        std::vector<std::string> keys;
        for(int i=0; i<params.size(); i++) {
            std::string p = "[" + std::to_string(i) + "]";
            //Check the expression contains the parameter
            if(expression.find(p) == std::string::npos) {
                throw std::runtime_error("Parameter " + p + " not found in expression");
            }
            keys.push_back(p);
            param_map[p] = params[i];
        }

        // Check the expression is contains time and charge
        if(expression.find("time") == std::string::npos) {
            throw std::runtime_error("Parameter [time] not found in expression");
        }
        if(expression.find("charge") == std::string::npos) {
            throw std::runtime_error("Parameter [charge] not found in expression");
        }

        auto& serviceSvc = algorithms::ServiceSvc::instance();
        m_evaluator = serviceSvc.service<EvaluatorSvc>("EvaluatorSvc")->_compile(expression, keys);
    };

    double operator()(double time, double charge) {
        param_map["time"] = time;
        param_map["charge"] = charge;
        return m_evaluator(param_map);
    }

    float getMaximumTime() const {
        return 0;
    }

private:
    std::unordered_map<std::string, double> param_map;
    std::function<double(const std::unordered_map<std::string, double>&)> m_evaluator;
};




class PulseShapeFactory {
    public:
        static std::unique_ptr<SignalPulse> createPulseShape(const std::string& type, const std::vector<double>& params) {
            if (type == "LandauPulse") {
                return std::make_unique<LandauPulse>(params);
            }
            //
            // Add more pulse shape variants here as needed
    
            // If type not found, try and make a function using the ElavulatorSvc
            try {
                return std::make_unique<EvaluatorPulse>(type,params);
            } catch (...) {
                throw std::invalid_argument("Unable to make pulse shape type: " + type);
            }
    
        }
    };

} // eicrecon
