// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <memory>
#include <spdlog/spdlog.h>
#include <JANA/JApplication.h>
#include "services/log/Log_service.h"
#include "SpdlogExtensions.h"

namespace eicrecon {
    class SpdlogMixin {
        /** Logger mixin
         *
         * @example:
         *      class MyFactory : JFactory, SpdlogMixin {
         *
         *          void Init() {
         *              InitLogger(GetApplication(), "MyPlugin:MyFactory");
         *
         *              // Logger is ready and can be used:
         *              m_log->info("MyFactory logger initialized");
         *          }
         *
         *          void Process(...) {
         *              m_log->trace("Using logger!");
         *          }
         *      };
         */
    public:
        /**
         * Initializes logger through current LogService
         * @param app - JApplication pointer, as obtained from GetApplication()
         * @param param_prefix - name of both logger and user parameter
         * @param default_level - optional - default log level, overrides default logging level
         *                          : trace, debug, info, warn, err, critical, off
         *
         * @remark: Each logger is cloned from spdlog::default_logger(). This allows to set the default level
         *          and output formatting on a global system level. But sometimes it is useful to provide
         *          default log level independently on what is set by the system. Again we are about DEFAULT value
         *          if no user flag is provided
         *
         * @example:
         *      InitLogger(GetApplication(), "BTRK:TrackerHits")           // Default log level is set the same as in system
         *      InitLogger(GetApplication(), "BTRK:TrackerHits", "info")   // By default log level is info
         *
         *  will create "BTRK:TrackerHits" logger and check -PBTRK:TrackerHits:LogLevel user parameter
         */
        void InitLogger(JApplication* app, const std::string &param_prefix, const std::string &default_level = "") {

            // Logger. Get plugin level sub-log
            m_log = app->GetService<Log_service>()->logger(param_prefix);

            // Get log level from user parameter or default
            std::string log_level_str = default_level.empty() ?         // did user provide default level?
                                        eicrecon::LogLevelToString(m_log->level()) :   //
                                        default_level;
            app->SetDefaultParameter(param_prefix + ":LogLevel", log_level_str, "LogLevel: trace, debug, info, warn, err, critical, off");
            m_log->set_level(eicrecon::ParseLogLevel(log_level_str));
        }

    public:
        std::shared_ptr<spdlog::logger> &logger() { return m_log; }

    protected: // FIXME change to private
        /// current logger
        std::shared_ptr<spdlog::logger> m_log;

    };
}
