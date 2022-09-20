#ifndef EICRECON_TRACKING_OCCUPANCY_PROCESSOR_H
#define EICRECON_TRACKING_OCCUPANCY_PROCESSOR_H

#include <TH1F.h>
#include <TH2F.h>
#include <TDirectory.h>

#include <JANA/JEventProcessor.h>
#include <JANA/JEventProcessorSequentialRoot.h>

#include "services/log/Log_service.h"

class JEvent;
class JApplication;

class TrackingEfficiency_processor:public JEventProcessor
{
public:
    explicit TrackingEfficiency_processor(JApplication *);
    ~TrackingEfficiency_processor() override = default;

    //----------------------------
    // Init
    //
    // This is called once before the first call to the Process method
    // below. You may, for example, want to open an output file here.
    // Only one thread will call this.
    void Init() override;

    //----------------------------
    // Process
    //
    // This is called for every event. Multiple threads may call this
    // simultaneously. If you write something to an output file here
    // then make sure to protect it with a mutex or similar mechanism.
    // Minimize what is done while locked since that directly affects
    // the multi-threaded performance.
    void Process(const std::shared_ptr<const JEvent>& event) override;

    //----------------------------
    // Finish
    //
    // This is called once after all events have been processed. You may,
    // for example, want to close an output file here.
    // Only one thread will call this.
    void Finish() override;

private:

    TDirectory* m_dir_main;               /// Main TDirectory for this plugin 'occupancy_ana'
    TH1F * m_th1_prt_pz;                  /// MC Particles pz
    TH1F * m_th1_prt_energy;              /// MC Particles total E
    TH1F * m_th1_prt_theta;               /// MC Particles theta angle
    TH1F * m_th1_prt_phi;                 /// MC Particles phi angle
    TH2F * m_th2_prt_pxy;                 /// MC Particles px,py

    std::shared_ptr<spdlog::logger> m_log;
};

#endif //EICRECON_TRACKING_OCCUPANCY_PROCESSOR_H
