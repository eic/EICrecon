// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//  Sections Copyright (C) 2022 Chao Peng, Wouter Deconinck, Sylvester Joosten
//  under SPDX-License-Identifier: LGPL-3.0-or-later

#include "JFactory_EcalBarrelRawCalorimeterHit.h"

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
// Constructor
//------------------------
JFactory_EcalBarrelRawCalorimeterHit::JFactory_EcalBarrelRawCalorimeterHit(){
    SetTag("");

    // This allows one to get the objects from this factory as edm4hep::RawCalorimeterHit.
    // This is useful for the EDM4hepWriter.
    EnableGetAs<edm4hep::RawCalorimeterHit>();
}

//------------------------
// Init
//------------------------
void JFactory_EcalBarrelRawCalorimeterHit::Init() {
    auto app = GetApplication();

    // I set the default values for the configuration parameters here, near the
    // calls to SetDefaultParameter. Mainly because that is what I'm used to.
    // The default values could also be set in the header file which would more
    // closely match the Gaudi style and may be a little cleaner.

    // TODO: There are 3 config values that are arrays that are not currently handled
    //Gaudi::Property<std::vector<double>> u_eRes{this, "energyResolutions", {}}; // a/sqrt(E/GeV) + b + c/(E/GeV)
    //Gaudi::Property<std::vector<std::string>> u_fields{this, "signalSumFields", {}};
    //Gaudi::Property<std::vector<int>>         u_refs{this, "fieldRefNumbers", {}};

    // additional smearing resolutions
    m_tRes = 0.0 * ns;
    japp->SetDefaultParameter("BarrelEMCal:timeResolution", m_tRes);

    // digitization settings
    m_capADC        = 8096;
    m_dyRangeADC    = 100 * MeV;
    m_pedMeanADC    = 400;
    m_pedSigmaADC   = 3.2;
    m_resolutionTDC = 10 * picosecond;
    m_corrMeanScale = 1.0;
    japp->SetDefaultParameter("BarrelEMCal:capacityADC",     m_capADC);
    japp->SetDefaultParameter("BarrelEMCal:dynamicRangeADC", m_dyRangeADC);
    japp->SetDefaultParameter("BarrelEMCal:pedestalMean",    m_pedMeanADC);
    japp->SetDefaultParameter("BarrelEMCal:pedestalSigma",   m_pedSigmaADC);
    japp->SetDefaultParameter("BarrelEMCal:resolutionTDC",   m_resolutionTDC);
    japp->SetDefaultParameter("BarrelEMCal:scaleResponse",   m_corrMeanScale);

    // signal sums
    m_geoSvcName = "GeoSvc";
    m_readout    = "";
    japp->SetDefaultParameter("BarrelEMCal:geoServiceName",  m_geoSvcName);
    japp->SetDefaultParameter("BarrelEMCal:readoutClass",    m_readout);

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
    for (size_t i = 0; i < u_eRes.size() && i < 3; ++i) {
        eRes[i] = u_eRes[i];
    }

    // using juggler internal units (GeV, mm, radian, ns)
    dyRangeADC = m_dyRangeADC / GeV;
    tRes       = m_tRes / ns;
    stepTDC    = ns / m_resolutionTDC;

    // need signal sum
    if (!u_fields.empty()) {
        m_geoSvc = app->GetService<JDD4hep_service>(); // TODO: implement named geometry service?
        // sanity checks
        if (!m_geoSvc) {
            LOG_ERROR(default_cerr_logger) << "Unable to locate Geometry Service. " << LOG_END;
            japp->Quit();
            return;
        }
        if (m_readout.empty()) {
            LOG_ERROR(default_cerr_logger) << "readoutClass is not provided, it is needed to know the fields in readout ids" << LOG_END;
            japp->Quit();
            return;
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
            LOG_ERROR(default_cerr_logger) << "Failed to load ID decoder for " << m_readout << LOG_END;
            japp->Quit();
            return;
        }
        id_mask = ~id_mask;
        LOG_INFO(default_cout_logger) << fmt::format("ID mask in {:s}: {:#064b}", m_readout, id_mask) << LOG_END;
    }
}

//------------------------
// ChangeRun
//------------------------
void JFactory_EcalBarrelRawCalorimeterHit::ChangeRun(const std::shared_ptr<const JEvent> &event) {
    /// This is automatically run before Process, when a new run number is seen
    /// Usually we update our calibration constants by asking a JService
    /// to give us the latest data for this run number
    
    auto run_nr = event->GetRunNumber();
    // m_calibration = m_service->GetCalibrationsForRun(run_nr);
}

//------------------------
// Process
//------------------------
void JFactory_EcalBarrelRawCalorimeterHit::Process(const std::shared_ptr<const JEvent> &event) {

    if (!u_fields.empty()) {
        signal_sum_digi(event);
    } else {
        single_hits_digi(event);
    }
}

//------------------------
// single_hits_digi
//------------------------
void JFactory_EcalBarrelRawCalorimeterHit::single_hits_digi( const std::shared_ptr<const JEvent> &event ){

    // Debugging: print all factories
    //for( auto fac : event->GetAllFactories() ) std::cout << fac->GetObjectName() <<" : " << fac->GetTag() <<" : " << fac->GetNumObjects() << std::endl;

    // input collections
    auto simhits = event->Get<edm4hep::SimCalorimeterHit>("EcalBarrelHits");

     // Create output collections
    // auto* rawhits = m_outputHitCollection.createAndPut();
    std::vector<EcalBarrelRawCalorimeterHit*> rawhits;
    for ( auto ahit : simhits ) {
        // Note: juggler internal unit of energy is GeV
        const double eDep    = ahit->getEnergy();

        // apply additional calorimeter noise to corrected energy deposit
        const double eResRel = (eDep > 1e-6)
                               ? m_normDist(generator) * std::sqrt(std::pow(eRes[0] / std::sqrt(eDep), 2) +
                                                          std::pow(eRes[1], 2) + std::pow(eRes[2] / (eDep), 2))
                               : 0;

        const double ped    = m_pedMeanADC + m_normDist(generator) * m_pedSigmaADC;
        const long long adc = std::llround(ped +  m_corrMeanScale * eDep * (1. + eResRel) / dyRangeADC * m_capADC);

        double time = std::numeric_limits<double>::max();
        for (const auto& c : ahit->getContributions()) {
            if (c.getTime() <= time) {
                time = c.getTime();
            }
        }
        const long long tdc = std::llround((time + m_normDist(generator) * tRes) * stepTDC);

        auto rawhit = new EcalBarrelRawCalorimeterHit(
                ahit->getCellID(),
                (adc > m_capADC ? m_capADC : adc),
                tdc
        );
        rawhits.push_back(rawhit);
    }
    Set( rawhits ); // publish to JANA
}

//------------------------
// signal_sum_digi
//------------------------
void JFactory_EcalBarrelRawCalorimeterHit::signal_sum_digi( const std::shared_ptr<const JEvent> &event ){

    auto simhits = event->Get<edm4hep::SimCalorimeterHit>("EcalBarrelHits");
    std::vector<EcalBarrelRawCalorimeterHit*> rawhits;

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
        double edep     = hits[0]->getEnergy();
        double time     = hits[0]->getContributions(0).getTime();
        double max_edep = hits[0]->getEnergy();
        // sum energy, take time from the most energetic hit
        for (size_t i = 1; i < hits.size(); ++i) {
            edep += hits[i]->getEnergy();
            if (hits[i]->getEnergy() > max_edep) {
                max_edep = hits[i]->getEnergy();
                for (const auto& c : hits[i]->getContributions()) {
                    if (c.getTime() <= time) {
                        time = c.getTime();
                    }
                }
            }
        }

        double eResRel = 0.;
        // safety check
        if (edep > 1e-6) {
            eResRel = m_normDist(generator) * eRes[0] / std::sqrt(edep) +
                      m_normDist(generator) * eRes[1] +
                      m_normDist(generator) * eRes[2] / edep;
        }
        double    ped     = m_pedMeanADC + m_normDist(generator) * m_pedSigmaADC;
        unsigned long long adc     = std::llround(ped + edep * (1. + eResRel) / dyRangeADC * m_capADC);
        unsigned long long tdc     = std::llround((time + m_normDist(generator) * tRes) * stepTDC);

        auto rawhit = new EcalBarrelRawCalorimeterHit(
                id,
                (adc > m_capADC ? m_capADC : adc),
                tdc
        );
        rawhits.push_back(rawhit);
    }
    Set( rawhits ); // publish to JANA
}
