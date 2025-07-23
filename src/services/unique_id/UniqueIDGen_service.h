#pragma once

#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/JServiceFwd.h>
#include <spdlog/logger.h>
#include <memory>
#include <string>

#include "algorithms/interfaces/UniqueIDGenSvc.h"
#include "services/log/Log_service.h"

class UniqueIDGen_service : public JService {
public:
  explicit UniqueIDGen_service(JApplication* app);
  ~UniqueIDGen_service() override {}

  void acquire_services(JServiceLocator* /* locator */) override {
    auto log_service = m_app->GetService<Log_service>();
    m_log            = log_service->logger("UniqueIDGen");
  }

  algorithms::UniqueIDGenSvc& getSvc() { return m_uniqueGenIDSvc; }

private:
  JApplication* m_app{nullptr};
  algorithms::UniqueIDGenSvc& m_uniqueGenIDSvc;
  std::shared_ptr<spdlog::logger> m_log;
};
