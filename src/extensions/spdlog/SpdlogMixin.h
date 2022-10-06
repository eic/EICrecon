// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#ifndef EICRECON_SPDLOGMIXIN_H
#define EICRECON_SPDLOGMIXIN_H

#include <memory>
#include <spdlog/spdlog.h>
#include <JANA/JApplication.h>
#include <services/log/Log_service.h>
#include "SpdlogExtensions.h"

namespace eicrecon {
    template <class T>
    class SpdlogMixin {
        /** Is it a bird? Is it a plane? No! It is CRTP logger mixin...
         *
         * How to use it?
         *
         * @example:
         *      class MyFactory : JFactory, SpdlogMixin<MyFactory> {
         *
         *          void Init() {
         *              InitLogger("MyPlugin:MyFactory", "info");
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
         * @param param_prefix - name of both logger and user parameter
         * @param default_level - default log level: trace, debug, info, warn, err, critical, off
         *
         * @example:
         *      InitLogger("BTRK:TrackerHits", "info")
         *
         *  will create "BTRK:TrackerHits" logger and check -PBTRK:TrackerHits:LogLevel user parameter
         */
        void InitLogger(const std::string &param_prefix, const std::string &default_level) {

            JApplication* app = static_cast<T*>(this)->GetApplication();

            // Logger. Get plugin level sub-log
            m_log = app->GetService<Log_service>()->logger(param_prefix);

            // Get log level from user parameter or default
            std::string log_level_str = default_level;
            app->SetDefaultParameter(param_prefix + ":LogLevel", log_level_str, "LogLevel: trace, debug, info, warn, err, critical, off");
            m_log->set_level(eicrecon::ParseLogLevel(log_level_str));
        }

    protected:
        /// current logger
        std::shared_ptr<spdlog::logger> m_log;

        std::shared_ptr<spdlog::logger> &logger() { return m_log; }
    };
}

#endif //EICRECON_SPDLOGMIXIN_H
