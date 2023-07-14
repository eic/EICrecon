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
            for (size_t i = 0; i < u_fields.size(); ++i) {
                id_mask |= id_desc.field(u_fields[i])->mask();
            }
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
std::unique_ptr<edm4hep::RawCalorimeterHitCollection> CalorimeterHitDigi::AlgorithmProcess(const edm4hep::SimCalorimeterHitCollection &simhits)  {
    if (merge_hits) {
        return std::move(signal_sum_digi(simhits));
    } else {
        return std::move(single_hits_digi(simhits));
    }
}

std::unique_ptr<edm4hep::RawCalorimeterHitCollection> CalorimeterHitDigi::single_hits_digi(const edm4hep::SimCalorimeterHitCollection &simhits)  {
    std::unique_ptr<edm4hep::RawCalorimeterHitCollection> rawhits { std::make_unique<edm4hep::RawCalorimeterHitCollection>() };

     // Create output collections
    for (const auto &ahit : simhits) {
        // Note: juggler internal unit of energy is dd4hep::GeV
        const double eDep    = ahit.getEnergy();

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
        for (const auto& c : ahit.getContributions()) {
            if (c.getTime() <= time) {
                time = c.getTime();
            }
        }
        if (time > m_capTime) continue;

        const long long tdc = std::llround((time + m_normDist(generator) * tRes) * stepTDC);

        if (eDep> 1.e-3) m_log->trace("E sim {} \t adc: {} \t time: {}\t maxtime: {} \t tdc: {} \t cell ID {}", eDep, adc, time, m_capTime, tdc, ahit.getCellID());
        rawhits->create(
                ahit.getCellID(),
                (adc > m_capADC ? m_capADC : adc),
                tdc
        );
    }

    return std::move(rawhits);
}

std::unique_ptr<edm4hep::RawCalorimeterHitCollection> CalorimeterHitDigi::signal_sum_digi(const edm4hep::SimCalorimeterHitCollection &simhits)  {
    auto rawhits = std::make_unique<edm4hep::RawCalorimeterHitCollection>();

    // find the hits that belong to the same group (for merging)
    std::unordered_map<uint64_t, std::vector<std::size_t> merge_map;
    std::size_t ix = 0;
    for (const auto &ahit : simhits) {
        uint64_t hid = ahit.getCellID() & id_mask;

        m_log->trace("org cell ID in {:s}: {:#064b}", m_readout, ahit.getCellID());
        m_log->trace("new cell ID in {:s}: {:#064b}", m_readout, hid);

        merge_map[hid].push_back(ix);

        ix++;
    }

    // signal sum
    // NOTE: we take the cellID of the most energetic hit in this group so it is a real cellID from an MC hit
    for (const auto &[id, ixs] : merge_map) {
        double edep     = 0;
        double time     = std::numeric_limits<double>::max();
        double max_edep = 0;
        auto   mid      = simhits[ixs[0]].getCellID();
        // sum energy, take time from the most energetic hit
        for (size_t i = 0; i < ixs.size(); ++i) {
            auto hit = simhits[ixs[i]];

            double timeC = std::numeric_limits<double>::max();
            for (const auto& c : hit.getContributions()) {
                if (c.getTime() <= timeC) {
                    timeC = c.getTime();
                }
            }
            if (timeC > m_capTime) continue;
            edep += hit.getEnergy();
            m_log->trace("adding {} \t total: {}", hit.getEnergy(), edep);

            // change maximum hit energy & time if necessary
            if (hit.getEnergy() > max_edep) {
                max_edep = hit.getEnergy();
                mid = hit.getCellID();
                for (const auto& c : hit.getContributions()) {
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
        rawhits->create(
                mid,
                (adc > m_capADC ? m_capADC : adc),
                tdc
        );
    }

    return std::move(rawhits);
}
