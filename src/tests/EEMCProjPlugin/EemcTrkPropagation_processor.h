#ifndef EICRECON_EEMC_TRK_PROPAGATION_PROCESSOR_H
#define EICRECON_EEMC_TRK_PROPAGATION_PROCESSOR_H

#include <JANA/JEventProcessor.h>
#include <JANA/JEventProcessorSequentialRoot.h>

#include <Acts/Surfaces/DiscSurface.hpp>

#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogMixin.h>
#include <algorithms/tracking/TrackPropagation.h>

#include <TDirectory.h>

#include <TH1.h>
#include <TH2.h>

class JEvent;
class JApplication;

class EemcTrkPropagation_processor:
        public JEventProcessor,
        public eicrecon::SpdlogMixin<EemcTrkPropagation_processor>   // this automates proper log initialization
{
public:
    explicit EemcTrkPropagation_processor(JApplication *);
    ~EemcTrkPropagation_processor() override = default;

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

    //Histograms
    TH2 *h1a; //Rec track momentum vs. true momentum
    TH2 *h1b; //EEMC cluster E vs. true energy
    TH1 *h2a; //Track projection z at EEMC
    TH1 *h2b; //EEMC Cluster z position
    TH2 *h3a; //EEMC Cluster x position vs. Track projection x
    TH2 *h3b; //EEMC Cluster y position vs. Track projection y
    TH1 *h4a; //EEMC Cluster x position minus Track projection x 
    TH1 *h4b; //EEMC Cluster y position minus Track projection y 
    TH1 *h5a; //EEMC cluster E divided by rec track momentum

private:

    /// Directory to store histograms to
    TDirectory *m_dir_main{};

    /// Tracking propagation algorithm
    eicrecon::TrackPropagation m_propagation_algo;

    /// A surface to propagate to
    std::shared_ptr<Acts::DiscSurface> m_ecal_surface;

};

#endif //EICRECON_EEMC_TRK_PROPAGATION_PROCESSOR_H
