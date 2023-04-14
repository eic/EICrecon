
#pragma once

#include <services/io/podio/JFactoryPodioT.h>

#include <algorithms/calorimetry/CalorimeterHitsMerger.h>
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>

class CalorimeterHit_factory_EcalBarrelScFiMergedHits : public eicrecon::JFactoryPodioT<edm4eic::CalorimeterHit>, CalorimeterHitsMerger {

public:
    //------------------------------------------
    // Constructor
    CalorimeterHit_factory_EcalBarrelScFiMergedHits(){
        SetTag("EcalBarrelScFiMergedHits");
        m_log = japp->GetService<Log_service>()->logger(GetTag());
    }

    //------------------------------------------
    // Init
    void Init() override{
        auto app = GetApplication();
        m_log = app->GetService<Log_service>()->logger(GetTag());

        m_input_tag = "EcalBarrelScFiRecHits";

        m_readout="EcalBarrelScFiHits";
        u_fields={"fiber","z"};
        u_refs={1,1};

        m_geoSvc= app->GetService<JDD4hep_service>();

        app->SetDefaultParameter("BEMC:EcalBarrelscFiMergedHits:input_tag", m_input_tag);
        app->SetDefaultParameter("BEMC:EcalBarrelscFiMergedHits:fields", u_fields);
        app->SetDefaultParameter("BEMC:EcalBarrelscFiMergedHits:refs",  u_refs);

        initialize();
    }

    //------------------------------------------
    // ChangeRun
    void ChangeRun(const std::shared_ptr<const JEvent> &event) override{}

    //------------------------------------------
    // Process
    void Process(const std::shared_ptr<const JEvent> &event) override{
        // Prefill inputs
        m_inputs = event->Get<edm4eic::CalorimeterHit>(m_input_tag);

        // Call Process for generic algorithm
        execute();

        // Hand ownership of algorithm objects over to JANA
        Set(m_outputs);
        m_outputs.clear(); // not really needed, but better to not leave dangling pointers around
    }

private:
    std::string m_input_tag;
};
