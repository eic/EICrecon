// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng, Wouter Deconinck, Sylvester Joosten, Barak Schmookler, David Lawrence

// A general digitization for CalorimeterHit from simulation
// 1. Smear energy deposit with a/sqrt(E/GeV) + b + c/E or a/sqrt(E/GeV) (relative value)
// 2. Digitize the energy with dynamic ADC range and add pedestal (mean +- sigma)
// 3. Time conversion with smearing resolution (absolute value)
// 4. Signal is summed if the SumFields are provided
//
// Author: Chao Peng
// Date: 06/02/2021


#include "CalorimeterHitDigi.h"

#include <DD4hep/Detector.h>
#include <DD4hep/IDDescriptor.h>
#include <DD4hep/Readout.h>
#include <DD4hep/config.h>
#include <DDSegmentation/BitFieldCoder.h>
#include <Evaluator/DD4hepUnits.h>
#include <algorithms/service.h>
#include <edm4eic/EDM4eicVersion.h>
#if EDM4EIC_VERSION_MAJOR >= 7
#include <edm4eic/MCRecoCalorimeterHitAssociationCollection.h>
#endif
#include <edm4hep/CaloHitContributionCollection.h>
#include <fmt/core.h>
#include <podio/RelationRange.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <gsl/pointers>
#include <limits>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "algorithms/calorimetry/CalorimeterHitDigiConfig.h"
#include "services/evaluator/EvaluatorSvc.h"

using namespace dd4hep;

namespace eicrecon {

//
// TODO:
// - Array type configuration parameters are not yet supported in JANA (needs to be added)
// - Random number service needs to bew resolved (on global scale)
// - It is possible standard running of this with Gaudi relied on a number of parameters
//   being set in the config. If that is the case, they should be moved into the default
//   values here. This needs to be confirmed.


void CalorimeterHitDigi::init() {

    // Gaudi implements a random number generator service. It is not clear to me how this
    // can work. There are multiple race conditions that occur in parallel event processing:
    // 1. The exact same events processed by a given thread in one invocation will not
    //    necessarily be the combination of events any thread sees in a subsequent
    //    invocation. Thus, you can't rely on thread_local storage.
    // 2. Its possible for the factory execution order to be modified by the presence of
    //    a processor (e.g. monitoring plugin). This is not as serious since changing the
    //    command line should cause one not to expect reproducibility. Still, one may
    //    expect the inclusion of an "observer" plugin not to have such side affects.
    //
    // More information will be needed. In the meantime, we implement a local random number
    // generator. Ideally, this would be seeded with the run number+event number, but for
    // now, just use default values defined in header file.

    // set energy resolution numbers
    if (m_cfg.eRes.empty()) {
      m_cfg.eRes.resize(3);
    } else if (m_cfg.eRes.size() != 3) {
      error("Invalid m_cfg.eRes.size()");
      throw std::runtime_error("Invalid m_cfg.eRes.size()");
    }

    // using juggler internal units (GeV, mm, radian, ns)
    tRes       = m_cfg.tRes / dd4hep::ns;
    stepTDC    = dd4hep::ns / m_cfg.resolutionTDC;

    // sanity checks
    if (m_cfg.readout.empty()) {
        error("readoutClass is not provided, it is needed to know the fields in readout ids");
        throw std::runtime_error("readoutClass is not provided");
    }

    // get decoders
    try {
        id_spec = m_geo.detector()->readout(m_cfg.readout).idSpec();
    } catch (...) {
        // Can not be more verbose. In JANA2, this will be attempted at each event, which
        // pollutes output for geometries that are less than complete.
        // We could save an exception and throw it from process.
        debug("Failed to load ID decoder for {}", m_cfg.readout);
        throw std::runtime_error(fmt::format("Failed to load ID decoder for {}", m_cfg.readout));
    }

    decltype(id_mask) id_inverse_mask = 0;
    // all these are for signal sum at digitization level
    if (!m_cfg.fields.empty()) {
        for (auto & field : m_cfg.fields) {
            id_inverse_mask |= id_spec.field(field)->mask();
        }
        debug("ID mask in {:s}: {:#064b}", m_cfg.readout, id_mask);
    }
    id_mask = ~id_inverse_mask;

    std::function hit_to_map = [this](const edm4hep::SimCalorimeterHit &h) {
      std::unordered_map<std::string, double> params;
      for(const auto &p : id_spec.fields()) {
        const std::string &name = p.first;
        const dd4hep::IDDescriptor::Field* field = p.second;
        params.emplace(name, field->value(h.getCellID()));
        trace("{} = {}", name, field->value(h.getCellID()));
      }
      return params;
    };

    auto& serviceSvc = algorithms::ServiceSvc::instance();
    corrMeanScale = serviceSvc.service<EvaluatorSvc>("EvaluatorSvc")->compile(m_cfg.corrMeanScale, hit_to_map);
}


void CalorimeterHitDigi::process(
      const CalorimeterHitDigi::Input& input,
      const CalorimeterHitDigi::Output& output) const {

    const auto [simhits] = input;
#if EDM4EIC_VERSION_MAJOR >= 7
    auto [rawhits, rawassocs] = output;
#else
    auto [rawhits] = output;
#endif

    // find the hits that belong to the same group (for merging)
    std::unordered_map<uint64_t, std::vector<std::size_t>> merge_map;
    std::size_t ix = 0;
    for (const auto &ahit : *simhits) {
        uint64_t hid = ahit.getCellID() & id_mask;

        trace("org cell ID in {:s}: {:#064b}", m_cfg.readout, ahit.getCellID());
        trace("new cell ID in {:s}: {:#064b}", m_cfg.readout, hid);

        merge_map[hid].push_back(ix);

        ix++;
    }

    // signal sum
    // NOTE: we take the cellID of the most energetic hit in this group so it is a real cellID from an MC hit
    for (const auto &[id, ixs] : merge_map) {

        // create hit and association in advance
        edm4hep::MutableRawCalorimeterHit rawhit;
#if EDM4EIC_VERSION_MAJOR >= 7
        std::vector<edm4eic::MutableMCRecoCalorimeterHitAssociation> rawassocs_staging;
#endif

        double edep     = 0;
        double time     = std::numeric_limits<double>::max();
        double max_edep = 0;
        auto   leading_hit = (*simhits)[ixs[0]];
        // sum energy, take time from the most energetic hit
        for (size_t i = 0; i < ixs.size(); ++i) {
            auto hit = (*simhits)[ixs[i]];

            double timeC = std::numeric_limits<double>::max();
            for (const auto& c : hit.getContributions()) {
                if (c.getTime() <= timeC) {
                    timeC = c.getTime();
                }
            }
            if (timeC > m_cfg.capTime) continue;
            edep += hit.getEnergy();
            trace("adding {} \t total: {}", hit.getEnergy(), edep);

            // change maximum hit energy & time if necessary
            if (hit.getEnergy() > max_edep) {
                max_edep = hit.getEnergy();
                leading_hit = hit;
                if (timeC <= time) {
                    time = timeC;
                }
            }

#if EDM4EIC_VERSION_MAJOR >= 7
            edm4eic::MutableMCRecoCalorimeterHitAssociation assoc;
            assoc.setRawHit(rawhit);
            assoc.setSimHit(hit);
            assoc.setWeight(hit.getEnergy());
            rawassocs_staging.push_back(assoc);
#endif
        }
        if (time > m_cfg.capTime) continue;

        // safety check
        const double eResRel = (edep > m_cfg.threshold)
                ? m_gaussian(m_generator) * std::sqrt(
                     std::pow(m_cfg.eRes[0] / std::sqrt(edep), 2) +
                     std::pow(m_cfg.eRes[1], 2) +
                     std::pow(m_cfg.eRes[2] / (edep), 2)
                  )
                : 0;
        double corrMeanScale_value = corrMeanScale(leading_hit);

        double ped = m_cfg.pedMeanADC + m_gaussian(m_generator) * m_cfg.pedSigmaADC;

        // Note: both adc and tdc values must be positive numbers to avoid integer wraparound
        unsigned long long adc = std::max(std::llround(ped + edep * corrMeanScale_value * (1.0 + eResRel) / m_cfg.dyRangeADC * m_cfg.capADC), 0LL);
        unsigned long long tdc = std::llround((time + m_gaussian(m_generator) * tRes) * stepTDC);

        if (edep> 1.e-3) trace("E sim {} \t adc: {} \t time: {}\t maxtime: {} \t tdc: {} \t corrMeanScale: {}", edep, adc, time, m_cfg.capTime, tdc, corrMeanScale_value);

        rawhit.setCellID(leading_hit.getCellID());
        rawhit.setAmplitude(adc > m_cfg.capADC ? m_cfg.capADC : adc);
        rawhit.setTimeStamp(tdc);
        rawhits->push_back(rawhit);

#if EDM4EIC_VERSION_MAJOR >= 7
        for (auto& assoc : rawassocs_staging) {
            assoc.setWeight(assoc.getWeight() / edep);
            rawassocs->push_back(assoc);
        }
#endif
    }
}

} // namespace eicrecon
