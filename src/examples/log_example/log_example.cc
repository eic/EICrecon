// Copyright 2022, Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include <memory>
#include <JANA/JEventProcessor.h>
#include <JANA/JEvent.h>

#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>
#include <extensions/spdlog/SpdlogMixin.h>

// This header is important if you want to print objects with custom overloaded ostr stream operator
// i.e. objects that knows how to print self when used with cout<<object
// Such objects are used sometimes, e.g. lib eigen matrices
// You don't need this header most of the time (until you see a compilation error)
// #include <spdlog/fmt/ostr.h>

// SpdLog mixin example
// This is the recommended way to deal with logger in your classes
//
class MyLoggingProcessor:
        public JEventProcessor,
        public eicrecon::SpdlogMixin<MyLoggingProcessor> {

public:
    MyLoggingProcessor() { SetTypeName(NAME_OF_THIS); }

    void Init() override {
        // Initialize logger
        InitLogger("log_example");
        m_log->info("Hello world!");
    }



    void Process(const std::shared_ptr<const JEvent>& event) override {
        /// The function is executed every event

    }

    void Finish() override {}
};


/// This class is used to demonstrate different logging aspects
/// Extended example
class LogServiceProcessor: public JEventProcessor {
private:
    std::shared_ptr<spdlog::logger> m_log;

public:
    LogServiceProcessor() { SetTypeName(NAME_OF_THIS); }
    
    void Init() override {
        /// Once in app lifetime function call

        auto app = GetApplication();

        // The service centralizes the use of spdlog and properly spawning logger
        auto log_service = app->GetService<Log_service>();

        // Loggers are created by name. This allows to create specialized loggers like "MyClassLogger"
        // or loggers with some context that might be shared between units like "tracking"
        m_log = log_service->logger("LogServiceProcessor");

        // Certainly the most of the time it is a one-liner:
        // m_log = app->GetService<Log_service>()->logger("LogServiceProcessor");

        // How log levels should be used?
        // trace    - something very verbose like each heat parameter
        // debug    - all information that is relevant for an expert to debug but should not be present outside of debugging
        // info     - something that will always (almost) get into the global log
        // warning  - something bad that needs attention but results are considered usable
        // error    - something bad making results probably unusable
        // critical - imminent software failure and termination

        // connect log level with plugin parameter
        std::string log_level_str = log_service->getDefaultLevelStr();

        // Ask service locator for parameter manager. We want to get this plugin parameters.
        auto pm = app->GetJParameterManager();
        pm->SetDefaultParameter("log_example:log-level", log_level_str, "log_level: trace, debug, info, warning");

        // At this point log_level_str is initialized with user provided flag value or the default value

        // convert input std::string to spdlog::level::level_enum and set logger level
        m_log->set_level(eicrecon::ParseLogLevel(log_level_str));

        // convert log level back to string
        m_log->info("Hello world! My log level is: '{}'", eicrecon::LogLevelToString(m_log->level()));

        // Log formatting examples:
        PrintFormatExamples();
    }

    void PrintFormatExamples() {

        m_log->info("Welcome to spdlog!");
        // [info] Welcome to spdlog!

        m_log->error("Some error message with arg: {}", 1);
        // [error] Some error message with arg: 1

        m_log->warn("Easy padding in numbers like {:08d}", 12);
        // [warning] Easy padding in numbers like 00000012

        m_log->critical("Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
        // [critical] Support for int: 42;  hex: 2a;  oct: 52; bin: 101010

        m_log->info("Support for floats number of digits {:03.2f}", 1.23456);
        // [info] Support for floats number of digits 1.23

        m_log->info("Positional args are {1} {0}...", "too", "supported");
        // [info] Positional args are supported too...

        m_log->info("{:>30}", "right aligned");
        // [info]                  right aligned

        m_log->info("Table of values:");
        m_log->info("{:<5} {:<7} {:>10}", "N", "val1", "val2");
        m_log->info("{:=^30}", "centered");  // use '=' as a fill char
        m_log->info("{:<5} {:<7.2f} {:>10}", 23, 3.1415, 3.1415);
        m_log->info("{:<5} {:<7.2f} {:>10}", 24, 2.7182, 2.7182);
        m_log->info("{:<5} {:<7.2f} {:>10}", 25, 1.6180, 1.6180);

        // [info] Table of values:
        // [info] N     val1          val2
        // [info] ===========centered===========
        // [info] 23    3.14        3.1415
        // [info] 24    2.72        2.7182
        // [info] 25    1.62         1.618

        m_log->debug("This message should not be displayed by default..");

        // Compile time log levels
        // define SPDLOG_ACTIVE_LEVEL to desired level
        // #define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
        SPDLOG_TRACE("Some trace message with param {}", 42);
        SPDLOG_DEBUG("Some debug message");
    }

    void Process(const std::shared_ptr<const JEvent>& event) override {
        /// The function is executed every event

        // In general one doesn't need to manually check log debug leel
        m_log->debug("Processing evnet #{}", event->GetEventNumber());

        // Sometimes one needs to know current log level to calculate debugging values
        // Use  <= operator to check level.
        // m_log->level() <= spdlog::level::debug  means 'debug' or 'trace'
        // m_log->level() <= spdlog::level::info   means 'info', 'debug' or 'trace' levels
        if(m_log->level() <= spdlog::level::debug) {
            int x = event->GetRunNumber()*1000000 + event->GetEventNumber()/2;
            m_log->debug("Calculated debug value #{}", x);
        }
    }

    void Finish() override {}
};


// The following just makes this a JANA plugin
extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);
        app->Add(new LogServiceProcessor);
        app->Add(new MyLoggingProcessor);
    }
}
    
