// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//
//

#include <JANA/JApplication.h>
#include <JANA/JFactoryGenerator.h>
#include <JANA/JEventProcessor.h>
#include <JANA/JEventProcessorSequentialRoot.h>
#include <JANA/JEvent.h>

#include <services/log/Log_service.h>
#include <services/integration/Algorithms_service.h>

#include <algorithms/calorimetry/ClusterRecoCoG.h>

#include <TDirectory.h>

class JEvent;
class JApplication;

class AlgorithmsTest_processor:public JEventProcessor
{
public:
    explicit AlgorithmsTest_processor(JApplication *);
    ~AlgorithmsTest_processor() override = default;

    //----------------------------
    // Init
    //
    // This is called once before the first call to the Process method
    // below. You may, for example, want to open an output file here.
    // Only one thread will call this.
    void Init() override {
        auto app = GetApplication();

        auto algo_svc = app->GetService<eicrecon::Algorithms_service>();
        auto &property_map = m_algo.getProperties();

        for (auto & [property_name, property] : property_map){
            fmt::print("{} {}\n", property_name);
            //app->SetDefaultParameter(std::string(property_name), property.get(), "hahaha");
        }
        m_algo.init();

    }

    //----------------------------
    // Process
    //
    // This is called for every event. Multiple threads may call this
    // simultaneously. If you write something to an output file here
    // then make sure to protect it with a mutex or similar mechanism.
    // Minimize what is done while locked since that directly affects
    // the multi-threaded performance.
    void Process(const std::shared_ptr<const JEvent>& event) override {


    }

    //----------------------------
    // Finish
    //
    // This is called once after all events have been processed. You may,
    // for example, want to close an output file here.
    // Only one thread will call this.
    void Finish() override {

    }

private:
    algorithms::calorimetry::ClusterRecoCoG m_algo;
    std::shared_ptr<spdlog::logger> m_log;
    TDirectory *m_dir_main;
};




extern "C" {
void InitPlugin(JApplication *app) {
    InitJANAPlugin(app);
    app->Add(new AlgorithmsTest_processor(app));
    //app->Add(new JFactoryGeneratorT<JFactory_EcalBarrelRawCalorimeterHit>());
    //app->Add(new JFactoryGeneratorT<JFactory_RawCalorimeterHit_EcalBarrelRawCalorimeterHits>());
}
}
    
