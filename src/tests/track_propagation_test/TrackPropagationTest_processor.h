#pragma once

#include <Acts/Surfaces/DiscSurface.hpp>
#include <JANA/JApplication.h>
#include <JANA/JEvent.h>
#include <JANA/JEventProcessor.h>
#include <TDirectory.h>
#include <memory>

#include "algorithms/tracking/TrackPropagation.h"
#include "extensions/spdlog/SpdlogMixin.h"

class TrackPropagationTest_processor:
        public JEventProcessor,
        public eicrecon::SpdlogMixin   // this automates proper log initialization
{
public:
    explicit TrackPropagationTest_processor(JApplication *);
    ~TrackPropagationTest_processor() override = default;

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

    /// Directory to store histograms to
    TDirectory *m_dir_main{};

    /// Tracking propagation algorithm
    eicrecon::TrackPropagation m_propagation_algo;

    /// A surface to propagate to
    std::shared_ptr<Acts::DiscSurface> m_hcal_surface;
};
