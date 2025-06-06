#pragma once

#include <JANA/JApplicationFwd.h>
#include <JANA/JServiceFwd.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <memory>
#include <mutex>
#include <optional>
#include <string>

/**
 * The Service centralizes use of spdlog
 */
class Log_service : public JService {
public:
  using level = spdlog::level::level_enum;

public:
  explicit Log_service(JApplication* app);
  ~Log_service();

  /** Get a named logger with optional level
     * When no level is specified, the service default is used **/
  virtual std::shared_ptr<spdlog::logger>
  logger(const std::string& name, const std::optional<level> default_level = std::nullopt);

  /** Gets the default level for all loggers
     * The log level is set from user parameters or is 'info'**/
  virtual level getDefaultLevel();

  /** Gets std::string version of the default log level **/
  virtual std::string getDefaultLevelStr();

private:
  Log_service() = default;

  std::recursive_mutex m_lock;
  JApplication* m_application;
  std::string m_log_level_str;
  std::string m_log_format_str;
};
