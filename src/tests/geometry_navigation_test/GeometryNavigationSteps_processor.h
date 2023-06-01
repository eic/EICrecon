#pragma once

#include <JANA/JEventProcessor.h>
#include <JANA/JEventProcessorSequentialRoot.h>
#include <Acts/Geometry/TrackingGeometry.hpp>
#include <Acts/MagneticField/MagneticFieldContext.hpp>

#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogMixin.h>

#include <TDirectory.h>

class JEvent;
class JApplication;

class GeometryNavigationSteps_processor:
        public JEventProcessor,
        public eicrecon::SpdlogMixin<GeometryNavigationSteps_processor>   // this automates proper log initialization
{
public:
    explicit GeometryNavigationSteps_processor(JApplication *);
    ~GeometryNavigationSteps_processor() override = default;

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
    Acts::GeometryContext m_geoContext;
    Acts::MagneticFieldContext m_fieldContext;
    std::shared_ptr<spdlog::logger> m_log;

};
