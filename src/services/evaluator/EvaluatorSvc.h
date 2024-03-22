// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Dmitry Kalinkin

#include <functional>
#include <string>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <vector>

#include <algorithms/logger.h>
#include <algorithms/service.h>

namespace eicrecon {

class EvaluatorSvc : public algorithms::LoggedService<EvaluatorSvc> {
public:
  void init();

  template <class... Args>
  std::function<double(Args...)>
  compile(const std::string& expr,
          std::function<std::unordered_map<std::string, double>(Args...)> transform) {
    std::vector<std::string> params;
    // Call transform with default values to detect parameter names
    for (auto& p : transform(Args{}...)) {
      params.push_back(p.first);
    }
    auto compiled_expr = _compile(expr, params);
    return [compiled_expr, transform](Args... args) {
      return compiled_expr(transform(std::forward<Args>(args)...));
    };
  };
  std::function<double(const std::unordered_map<std::string, double>&)>
  _compile(const std::string& expr, std::vector<std::string> params);

private:
  unsigned int m_function_id = 0;
  std::mutex m_interpreter_mutex;

  ALGORITHMS_DEFINE_LOGGED_SERVICE(EvaluatorSvc);
};

} // namespace eicrecon
