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
class Spdlog_service : public JService
{
public:
    explicit Spdlog_service( JApplication *app ): m_application(app){}

    std::shared_ptr<spdlog::logger> makeLogger(const std::string &name);

    std::shared_ptr<spdlog::logger> getLogger(const std::string &name);

protected:
    void Initialize();

private:
    Spdlog_service()=default;
    JApplication *m_application;
};

#endif // __Spdlog_service_h__
