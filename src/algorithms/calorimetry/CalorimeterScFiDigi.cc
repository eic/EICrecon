// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022 Chao Peng, Wouter Deconinck, Sylvester Joosten, Barak Schmookler, David Lawrence

// A digitization digitization algorithm specifically for CalorimeterHit from Scintillating Fibers
// 1. Fiber signals are merged if they are within the same light guide
// 2. Digitize the energy with dynamic ADC range and add pedestal (mean +- sigma)
// 3. Time conversion with smearing resolution (absolute value)
// TODO: Sum with timing; waveform implementation
//
// Author: Chao Peng
// Date: 02/12/2023


#include "CalorimeterScFiDigi.h"

#include <JANA/JEvent.h>
#include <edm4hep/SimCalorimeterHit.h>
#include <Evaluator/DD4hepUnits.h>
#include <fmt/format.h>
using namespace dd4hep;

//
// TODO:
// - Array type configuration parameters are not yet supported in JANA (needs to be added)
// - Random number service needs to be resolved (on global scale)



//------------------------
// AlgorithmInit
//------------------------
void CalorimeterScFiDigi::AlgorithmInit(std::shared_ptr<spdlog::logger>& logger) {

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
    for (size_t i = 0; i < u_eRes.size() && i < 3; ++i) {
        eRes[i] = u_eRes[i];
    }

    // using juggler internal units (GeV, mm, radian, ns)
    tRes       = m_tRes / dd4hep::ns;
    stepTDC    = dd4hep::ns / m_resolutionTDC;

    // need signal sum
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
            id_dec = id_desc.decoder();
            id_mask = 0;
            std::vector<std::pair<std::string, int>> ref_fields;
            for (size_t i = 0; i < u_fields.size(); ++i) {
                id_mask |= id_desc.field(u_fields[i])->mask();
                // use the provided id number to find ref cell, or use 0
                int ref = i < u_refs.size() ? u_refs[i] : 0;
                ref_fields.emplace_back(u_fields[i], ref);
            }
            // need to also merge z
            if (!m_zsegment.empty()) {
                z_idx = id_dec->index(m_zsegment);
                // treat z segment as a field to merge
                id_mask |= id_desc.field(m_zsegment)->mask();
                // but new z value will be determined in merging, 0 is a placeholder here
                ref_fields.emplace_back(m_zsegment, 0);
            }
            ref_mask = id_desc.encode(ref_fields);
            // debug() << fmt::format("Referece id mask for the fields {:#064b}", ref_mask) << endmsg;
            // update z field index from the decoder
        } catch (...) {
            m_log->warn("Failed to load ID decoder for {}", m_readout);
            japp->Quit();
            return;
        }
        id_mask = ~id_mask;
        //LOG_INFO(default_cout_logger) << fmt::format("ID mask in {:s}: {:#064b}", m_readout, id_mask) << LOG_END;
        m_log->info("ID mask in {:s}: {:#064b}", m_readout, id_mask);
    }
}



//------------------------
// AlgorithmChangeRun
//------------------------
void CalorimeterScFiDigi::AlgorithmChangeRun() {
    /// This is automatically run before Process, when a new run number is seen
    /// Usually we update our calibration constants by asking a JService
    /// to give us the latest data for this run number
}

//------------------------
// AlgorithmProcess
//------------------------
void CalorimeterScFiDigi::AlgorithmProcess()  {

    // Delete any output objects left from last event.
    // (Should already have been done for us, but just to be bullet-proof.)
    for( auto h : rawhits ) delete h;
    rawhits.clear();

    light_guide_digi();
}

//------------------------
// light_guide_digi
//------------------------
void CalorimeterScFiDigi::light_guide_digi( void ){

    // find the hits that belong to the same group (for merging)
    std::unordered_map<long long, std::vector<const edm4hep::SimCalorimeterHit*>> merge_map;
    for (auto ahit : simhits) {
        int64_t hid = (ahit->getCellID() & id_mask) | ref_mask;
        auto    it  = merge_map.find(hid);

        if (it == merge_map.end()) {
            merge_map[hid] = {ahit};
        } else {
            it->second.push_back(ahit);
        }
    }

    // signal sum
    for (auto &[id, hits] : merge_map) {
        double  edep     = hits[0]->getEnergy();
        double  time     = hits[0]->getContributions(0).getTime();
        double  max_edep = hits[0]->getEnergy();
        int64_t mid      = hits[0]->getCellID();

        // sum energy, take time from the most energetic hit
        // TODO, implement a timing window to sum or split the hits group
        for (size_t i = 1; i < hits.size(); ++i) {
            int64_t ztmp  = id_dec->get(hits[i]->getCellID(), z_idx);
            edep += hits[i]->getEnergy();
            if (hits[i]->getEnergy() > max_edep) {
                max_edep = hits[i]->getEnergy();
                mid = hits[i]->getCellID();
                for (const auto& c : hits[i]->getContributions()) {
                    if (c.getTime() <= time) {
                        time = c.getTime();
                    }
                }
            }
        }

        // safety check
        const double eResRel = (edep > m_threshold)
                ? m_normDist(generator) * eRes[0] / std::sqrt(edep) +
                  m_normDist(generator) * eRes[1] +
                  m_normDist(generator) * eRes[2] / edep
                  : 0;
        // digitize
        double    ped     = m_pedMeanADC + m_normDist(generator) * m_pedSigmaADC;
        unsigned long long adc     = std::llround(ped + edep * (m_corrMeanScale + eResRel) / m_dyRangeADC * m_capADC);
        unsigned long long tdc     = std::llround((time + m_normDist(generator) * tRes) * stepTDC);

        // use the cellid from the most energetic hit in this group
        auto rawhit = new edm4hep::RawCalorimeterHit(
                mid,
                (adc > m_capADC ? m_capADC : adc),
                tdc
        );
        rawhits.push_back(rawhit);
    }
    m_log->debug("size before digi: {:d}, after: {:d}", simhits.size(), rawhits.size());
}
