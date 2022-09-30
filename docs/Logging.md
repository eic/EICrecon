# Logging

EICrecon uses [spdlog](https://github.com/gabime/spdlog) library as a logging backbone. **spdlog** utilizes
[fmt](https://github.com/fmtlib/fmt) - a string formatting and output library which uses python alike string formatting
([is C++20 standard](https://en.cppreference.com/w/cpp/utility/format)) and is very performant
(in many cases [is faster than printf and std streams](https://github.com/fmtlib/fmt#speed-tests))


### Basic usage

EICRecon has a log service that centralizes default logger configuration and helps spawn named loggers.
Each unit - a plugin, factory, class, etc. can spawn its own named logger and use it:

[FULL WORKING EXAMPLE](https://github.com/eic/EICrecon/blob/main/src/examples/log_example/log_example.cc)

```c++
#include <services/log/Log_service.h>
class ExampleProcessor: public JEventProcessor {
private:
    std::shared_ptr<spdlog::logger> m_log;

public:
    /// Once in m_app lifetime function call
    void Init() override {        

        auto m_app = GetApplication();

        // The service centralizes the use of spdlog and properly spawning logger
        auto log_service = m_app->GetService<Log_service>();

        // Loggers are spawned by name.        
        m_log = log_service->logger("ExampleProcessor");
        
        // log things!
        m_log->info("Hello world! {}", 42);
    }
    
    /// The function is executed every event
    void Process(const std::shared_ptr<const JEvent>& event) override {
        // Will print something if 'trace' log level is set (see below)
        m_log->trace("Processing event #{}", event->GetEventNumber());
        // ...
    }
```


### Formatting example

Thanks to fmt, logger has rich text formatting. More examples and full
specification is in [fmt documentation](https://github.com/fmtlib/fmt):

```c++
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

// Compile time log levels
// define SPDLOG_ACTIVE_LEVEL to desired level e.g.:
//    #define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
SPDLOG_TRACE("Some trace message with param {}", 42);
SPDLOG_DEBUG("Some debug message");
```

### Log level

In order to wire your logger level with a jana-parameter to change log level without recompilation, use:

```c++
// includes: 
#include <services/log/Log_service.h>
#include <extensions/spdlog/SpdlogExtensions.h>

// ... all from previous example
void Init() override {
   // ...
   
   // This is a default level  
   std::string log_level_str = "info";
   
   // Ask service locator for parameter manager. We want to get this plugin parameters.
   auto pm = m_app->GetJParameterManager();
   
   // Define parameter
   pm->SetDefaultParameter("log_example:LogLevel", log_level_str, "log_level: trace, debug, info, warn, err, critical, off");
   
   // At this point log_level_str is initialized with user provided flag value or the default value
   
   // convert input std::string to spdlog::level::level_enum and set logger level
   m_log->set_level(eicrecon::ParseLogLevel(log_level_str));
}
```

**How log levels should be used?**

- **trace**    - something very verbose like each hit parameter
- **debug**    - all information that is relevant for an expert to debug but should not be present in production
- **info**     - something that will always (almost) get into the global log
- **warning**  - something that needs attention but results are likely usable
- **error**    - something bad making results probably unusable
- **critical** - imminent software failure and termination

Sometimes one needs to know current log level to calculate debugging values. Use **<=** operator to check level.
It works because enum values are: trace=0, debug=1, ... critical= 5. So:

```c++
  // In general one doesn't need to manually check log debug level
  m_log->debug("Will be printed if level is 'debug' or 'trace' ");

  // But sometimes one needs to know current log level to calculate debugging values
  // Use  <= operator to check level.
  // m_log->level() <= spdlog::level::debug  means 'debug' or 'trace'
  // m_log->level() <= spdlog::level::info   means 'info', 'debug' or 'trace' levels
  if(m_log->level() <= spdlog::level::debug) {
      int x = event->GetRunNumber()*1000000 + event->GetEventNumber()/2;
      m_log->debug("Calculated debug value #{}", x);
  }
```

### SpdlogMixin

Since getting a log from service and setting log level is all-the-same boilerplate, there is a SpdlogMixin 
made for convenience. SpdlogMixin works with any class, that has GetApplication() method. 

SpdlogMixin provides: 
1. `std::shared_ptr<spdlog::logger> m_log;` - protected member.
2. `InitLogger(string param_prefix, string default_level)` - Initializes m_log through the standard (for EICrecon) procedure
   - `param_prefix` defines logger name. User parameter name will be "<param_prefix>:LogLevel"
   - `default_level` - default logger level if user didn't use a parameter(flag)


```cpp
#include <extensions/spdlog/SpdlogMixin.h>


class MyFactory : SpdlogMixin<MyFactory>, JFactory {    // CRTP template mixin
    void Init() {
        
        // Will initialize `m_log` logger with name "MyPlugin:MyFactory"  
        // and check "-PBTRK:TrackerHits:LogLevel" user parameter for log level
        // If user parameter is not set, "info" will be set as log level
        InitLogger("MyPlugin:MyFactory", "info");
        
        // Logger is ready and can be used:
        m_log->info("MyFactory logger initialized");
    }

    void Process(...) {
        m_log->trace("Using logger!");
    }
};
```

### Logging hints


#### Streamable objects

Sometimes you need to print objects with custom overloaded ostr stream operator `<<`
i.e. objects that knows how to print self when used with something like  ```cout<<object```
Such objects are used sometimes, e.g. lib eigen matrices might be printed that way.
To enable printing of such objects include the next header

```c++
// Include this to print/format streamable objects.
// You don't need this header most of the time (until you see a compilation error)
#include <spdlog/fmt/ostr.h>
```

#### Default logger

spdlog has a default logger and global functions like `spdlog::info`, `spdlog::warn` etc.

```c++
spdlog::info("Hello world from default logger! You can do this, but please don't");
```

It is possible to use a default logger to print something out, but it is NOT RECOMMENDED to be used instead
of a named loggers. Default logger is used to highlight something that relates to whole application execution.
Otherwise, use named logger.


#### Shared names

By default, spdlog fails if a logger with such name exists (but one can get existing logger
from registry). EICrecon Logger service simplifies and automates it with a single function `logger(name)`.
This allows to use the same logger with the same name from different units if the context is the same.
Imagine you want to highlight that this message belongs to "tracking" you can do:

```c++

// One class
m_tracking_log = m_app->GetService<Log_service>()->logger("tracking");

// Another class
m_tracking_log = m_app->GetService<Log_service>()->logger("tracking");
```

You can mix named loggers depending on context

```c++

// Some class INIT
m_this_log = m_app->GetService<Log_service>()->logger("ExampleFactoryName");
m_tracking_log = m_app->GetService<Log_service>()->logger("tracking");

// Some class event PROCESSING
m_this_log->trace("Something related to this class/factory/plugin");
m_tracking_log->info("Something relating to tracking in general");
```

#### String format

It is recommended to use fmt string formatting both from performance and safety points. But it is also very convenient!
fmt can be accessed through spdlog headers:

```c++
#include <spdlog/fmt/fmt.h>

// code
std::string hello = fmt::format("Hello world {}", 42);
```

#### CMake

spdlog is included by default in every plugin if `plugin_add` macro is used:

```cmake
plugin_add(${PLUGIN_NAME})  # <= spdlog will be included by default
```

eicrecon application also includes Log_service by default. So it should not appear on `-Pplugins` list.


### Logging links

- [spdlog](https://github.com/gabime/spdlog)
- [fmt](https://github.com/fmtlib/fmt)
- [EICrecon logging examples](https://github.com/eic/EICrecon/blob/main/src/examples/log_example/log_example.cc)
- [EICrecon factory with log example](https://github.com/eic/EICrecon/blob/main/src/algorithms/tracking/TrackerHitReconstruction_factory.cc)
