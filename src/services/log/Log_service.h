#pragma once


#include <JANA/JApplication.h>
#include <JANA/Services/JServiceLocator.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <memory>
#include <mutex>
#include <string>

/**
 * The Service centralizes use of spdlog
 */
class Log_service : public JService
{
public:
    explicit Log_service(JApplication *app);
    ~Log_service();

    virtual std::shared_ptr<spdlog::logger> logger(const std::string &name);

    /** Gets the default level for all loggers
     * The log level is set from user parameters or is 'info'**/
    virtual spdlog::level::level_enum getDefaultLevel();

    /** Gets std::string version of the default log level **/
    virtual std::string getDefaultLevelStr();


private:

    Log_service()=default;

    std::recursive_mutex m_lock;
    JApplication* m_application;
    std::string m_log_level_str;
    std::string m_log_format_str;
};
