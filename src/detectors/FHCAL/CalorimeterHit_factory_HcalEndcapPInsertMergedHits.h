
#pragma once

#include <services/io/podio/JFactoryPodioT.h>

#include <algorithms/calorimetry/CalorimeterHitsMerger.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>

class CalorimeterHit_factory_HcalEndcapPInsertMergedHits : public eicrecon::JFactoryPodioT<edm4eic::CalorimeterHit>, CalorimeterHitsMerger {

public:
    //------------------------------------------
    // Constructor
    CalorimeterHit_factory_HcalEndcapPInsertMergedHits(){
        SetTag("HcalEndcapPInsertMergedHits");
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();
        m_log = app->GetService<Log_service>()->logger(GetTag());

        m_input_tag = "HcalEndcapPInsertRecHits";

        m_readout="HcalEndcapPInsertHits";
        u_fields={"layer", "slice"};  // from ATHENA's reconstruction.py
        u_refs={1, 0};                // from ATHENA's reconstruction.py

        m_geoSvc= app->GetService<JDD4hep_service>();

        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertMergedHits:input_tag", m_input_tag);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertMergedHits:fields", u_fields);
        app->SetDefaultParameter("FHCAL:HcalEndcapPInsertMergedHits:refs",  u_refs);

        initialize();
    }

    //------------------------------------------
    // ChangeRun
    void ChangeRun(const std::shared_ptr<const JEvent> &event) override{}

    //------------------------------------------
    // Process
    void Process(const std::shared_ptr<const JEvent> &event) override{
        // Get input collection
        auto hits_coll = static_cast<const edm4eic::CalorimeterHitCollection*>(event->GetCollectionBase(m_input_tag));

        // Call Process for generic algorithm
        auto mergedhits_coll = execute(*hits_coll);

        // Hand algorithm objects over to JANA
        SetCollection(std::move(mergedhits_coll));
    }

private:
    std::string m_input_tag;
};
