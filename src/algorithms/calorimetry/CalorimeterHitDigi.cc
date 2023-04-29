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

#include <JANA/JEvent.h>
#include <edm4hep/SimCalorimeterHit.h>
#include <Evaluator/DD4hepUnits.h>
#include <fmt/format.h>
using namespace dd4hep;

//
// This algorithm converted from:
//
//  https://eicweb.phy.anl.gov/EIC/juggler/-/blob/master/JugDigi/src/components/CalorimeterHitDigi.cpp
//
// TODO:
// - Array type configuration parameters are not yet supported in JANA (needs to be added)
// - Random number service needs to bew resolved (on global scale)
// - It is possible standard running of this with Gaudi relied on a number of parameters
//   being set in the config. If that is the case, they should be moved into the default
//   values here. This needs to be confirmed.



//------------------------
// AlgorithmInit
//------------------------
void CalorimeterHitDigi::AlgorithmInit(std::shared_ptr<spdlog::logger>& logger) {

    // Assume all configuration parameter data members have been filled in already.

    // Gaudi implments a random number generator service. It is not clear to me how this
    // can work. There are multiple race conditions that occur in parallel event processing:
    // 1. The exact same events processed by a given thread in one invocation will not
    //    neccessarily be the combination of events any thread sees in a subsequest
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
    m_log=logger;

    if (u_eRes.size() == 0) {
      u_eRes.resize(3);
    } else if (u_eRes.size() != 3) {
      m_log->error("Invalid u_eRes.size()");
      throw std::runtime_error("Invalid u_eRes.size()");
    }

    // using juggler internal units (GeV, mm, radian, ns)
    tRes       = m_tRes / dd4hep::ns;
    stepTDC    = dd4hep::ns / m_resolutionTDC;

    // all these are for signal sum at digitization level
    merge_hits = false;
    if (!u_fields.empty()) {
        // sanity checks
        if (!m_geoSvc) {
            m_log->error("Unable to locate Geometry Service.");
            throw std::runtime_error("Unable to locate Geometry Service.");
        }
        if (m_readout.empty()) {
            m_log->error("readoutClass is not provided, it is needed to know the fields in readout ids.");
            throw std::runtime_error("readoutClass is not provided.");
        }

        // get decoders
        try {
            auto id_desc = m_geoSvc->detector()->readout(m_readout).idSpec();
            id_mask = 0;
            std::vector<std::pair<std::string, int>> ref_fields;
            for (size_t i = 0; i < u_fields.size(); ++i) {
                id_mask |= id_desc.field(u_fields[i])->mask();
                // use the provided id number to find ref cell, or use 0
                int ref = i < u_refs.size() ? u_refs[i] : 0;
                ref_fields.emplace_back(u_fields[i], ref);
            }
            ref_mask = id_desc.encode(ref_fields);
            // debug() << fmt::format("Referece id mask for the fields {:#064b}", ref_mask) << endmsg;
        } catch (...) {
            // a workaround to avoid breaking the whole analysis if a field is not in some configurations
            // TODO: it should be a fatal error to not cause unexpected analysis results
            m_log->warn("Failed to load ID decoder for {}, hits will not be merged.", m_readout);
            // throw::runtime_error(fmt::format("Failed to load ID decoder for {}", m_readout));
            return;
        }
        id_mask = ~id_mask;
        m_log->info("ID mask in {:s}: {:#064b}", m_readout, id_mask);
        // all checks passed
        merge_hits = true;
    }
}



//------------------------
// AlgorithmChangeRun
//------------------------
void CalorimeterHitDigi::AlgorithmChangeRun() {
    /// This is automatically run before Process, when a new run number is seen
    /// Usually we update our calibration constants by asking a JService
    /// to give us the latest data for this run number
}

//------------------------
// AlgorithmProcess
//------------------------
void CalorimeterHitDigi::AlgorithmProcess()  {

    // Delete any output objects left from last event.
    // (Should already have been done for us, but just to be bullet-proof.)
    for( auto h : rawhits ) delete h;
    rawhits.clear();

    if (merge_hits) {
        signal_sum_digi();
    } else {
        single_hits_digi();
    }
}

//------------------------
// single_hits_digi
//------------------------
void CalorimeterHitDigi::single_hits_digi(){

     // Create output collections
    for ( auto ahit : simhits ) {
        // Note: juggler internal unit of energy is dd4hep::GeV
        const double eDep    = ahit->getEnergy();

        // apply additional calorimeter noise to corrected energy deposit
        const double eResRel = (eDep > m_threshold)
                               ? m_normDist(generator) * std::sqrt(
                                    std::pow(u_eRes[0] / std::sqrt(eDep), 2) +
                                    std::pow(u_eRes[1], 2) +
                                    std::pow(u_eRes[2] / (eDep), 2)
                )
                               : 0;
//       const double eResRel = (eDep > 1e-6)
//                               ? m_normDist(generator) * std::sqrt(std::pow(u_eRes[0] / std::sqrt(eDep), 2) +
//                                                          std::pow(u_eRes[1], 2) + std::pow(u_eRes[2] / (eDep), 2))
//                               : 0;

        const double ped    = m_pedMeanADC + m_normDist(generator) * m_pedSigmaADC;
        const long long adc = std::llround(ped + eDep * (m_corrMeanScale + eResRel) / m_dyRangeADC * m_capADC);

        double time = std::numeric_limits<double>::max();
        for (const auto& c : ahit->getContributions()) {
            if (c.getTime() <= time) {
                time = c.getTime();
            }
        }
        if (time > m_capTime) continue;

        const long long tdc = std::llround((time + m_normDist(generator) * tRes) * stepTDC);

        if (eDep> 1.e-3) m_log->trace("E sim {} \t adc: {} \t time: {}\t maxtime: {} \t tdc: {} \t cell ID {}", eDep, adc, time, m_capTime, tdc, ahit->getCellID());
        auto rawhit = new edm4hep::RawCalorimeterHit(
                ahit->getCellID(),
                (adc > m_capADC ? m_capADC : adc),
                tdc
        );
        rawhits.push_back(rawhit);
    }
}

//------------------------
// signal_sum_digi
//------------------------
void CalorimeterHitDigi::signal_sum_digi( void ){

    // find the hits that belong to the same group (for merging)
    std::unordered_map<long long, std::vector<const edm4hep::SimCalorimeterHit*>> merge_map;
    for (auto ahit : simhits) {
        int64_t hid = (ahit->getCellID() & id_mask) | ref_mask;

        m_log->trace("org cell ID in {:s}: {:#064b}", m_readout, ahit->getCellID());
        m_log->trace("new cell ID in {:s}: {:#064b}", m_readout, hid);

        auto    it  = merge_map.find(hid);

        if (it == merge_map.end()) {
            merge_map[hid] = {ahit};
        } else {
            it->second.push_back(ahit);
        }
    }

    // signal sum
    // NOTE: we take the cellID of the most energetic hit in this group so it is a real cellID from an MC hit
    for (auto &[id, hits] : merge_map) {
        double edep     = 0;
        double time     = std::numeric_limits<double>::max();
        double max_edep = 0;
        auto   mid      = hits[0]->getCellID();
        // sum energy, take time from the most energetic hit
        m_log->trace("id: {} \t {}", id, edep);
        for (size_t i = 0; i < hits.size(); ++i) {

            double timeC = std::numeric_limits<double>::max();
            for (const auto& c : hits[i]->getContributions()) {
                if (c.getTime() <= timeC) {
                    timeC = c.getTime();
                }
            }
            if (timeC > m_capTime) continue;
            edep += hits[i]->getEnergy();
            m_log->trace("adding {} \t total: {}", hits[i]->getEnergy(), edep);

            // change maximum hit energy & time if necessary
            if (hits[i]->getEnergy() > max_edep) {
                max_edep = hits[i]->getEnergy();
                mid = hits[i]->getCellID();
                for (const auto& c : hits[i]->getContributions()) {
                    if (c.getTime() <= time) {
                        time = c.getTime();
                    }
                }
                if (timeC <= time) {
                    time = timeC;
                }
            }
        }

//        double eResRel = 0.;
        // safety check
        const double eResRel = (edep > m_threshold)
                ? m_normDist(generator) * u_eRes[0] / std::sqrt(edep) +
                  m_normDist(generator) * u_eRes[1] +
                  m_normDist(generator) * u_eRes[2] / edep
                  : 0;
//        if (edep > 1e-6) {
//            eResRel = m_normDist(generator) * u_eRes[0] / std::sqrt(edep) +
//                      m_normDist(generator) * u_eRes[1] +
//                      m_normDist(generator) * u_eRes[2] / edep;
//        }
        double    ped     = m_pedMeanADC + m_normDist(generator) * m_pedSigmaADC;
        unsigned long long adc     = std::llround(ped + edep * (m_corrMeanScale + eResRel) / m_dyRangeADC * m_capADC);
        unsigned long long tdc     = std::llround((time + m_normDist(generator) * tRes) * stepTDC);

        if (edep> 1.e-3) m_log->trace("E sim {} \t adc: {} \t time: {}\t maxtime: {} \t tdc: {}", edep, adc, time, m_capTime, tdc);
        auto rawhit = new edm4hep::RawCalorimeterHit(
                mid,
                (adc > m_capADC ? m_capADC : adc),
                tdc
        );
        rawhits.push_back(rawhit);
    }
}
