#ifndef __Spdlog_service_h__
#define __Spdlog_service_h__


#include <iostream>
#include <vector>
#include <string>

#include <JANA/JApplication.h>
#include <JANA/Services/JServiceLocator.h>

#include <spdlog/spdlog.h>

/**
 * The Service centralizes use of spdlog
 */
class Log_service : public JService
{
public:
    explicit Log_service(JApplication *app );

    std::shared_ptr<spdlog::logger> logger(const std::string &name);


private:

    Log_service()=default;

    std::recursive_mutex m_lock;
    JApplication* m_application;
    std::string m_log_level_str;
};

#endif // __Spdlog_service_h__
