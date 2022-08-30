// Copyright 2022, David Lawrence
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <memory>
#include <JANA/JEventProcessor.h>
#include <services/log/Spdlog_service.h>


/// This class is used to demonstrate different logging techniques
class LogServiceProcessor: public JEventProcessor {
private:
    std::shared_ptr<spdlog::logger> m_log;


public:
    LogServiceProcessor() { SetTypeName(NAME_OF_THIS); }
    
    void Init() override {

        //The service centralizes the use of spdlog and its
        auto log_service = GetApplication()->GetService<Spdlog_service>();
        m_log = log_service->getLogger("LogServiceProcessor")




    }

    void Process(const std::shared_ptr<const JEvent>& event) override {

        // Fill histograms
        for( auto hit : rawhits()  ) hEraw->Fill(  hit->getEnergy());
        for( auto hit : digihits() ) hEdigi->Fill( hit->getAmplitude());
    }

    void FinishWithGlobalRootLock() override {}
};

// The following just makes this a JANA plugin
extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);
        app->Add(new BEMC_testProcessor);
    }
}
    
