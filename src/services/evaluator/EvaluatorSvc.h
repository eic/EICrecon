// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Dmitry Kalinkin

#include <algorithms/logger.h>
#include <exception>
#include <functional>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace eicrecon {

/**
 * @brief Provides an interface to a compiler that converts string expressions
 * to native `std::function`.
 *
 * The underlying implementation relies on ROOT's TInterpreter, but this may
 * change in the future. User can inspect the full C++ code by setting
 * `-PEvaluatorSvc:LogLevel=debug`, the list of provided variables is apparent
 * from the same output.
 *
 * Currently, return type is fixed to `double`, and all input parameters have
 * to be convertible to double.
 */
class EvaluatorSvc : public algorithms::LoggedService<EvaluatorSvc> {
public:
  void init();

  /**
   * @brief Compile expression `expr` to std::function
   * @param expr String expression to compile (e.g. `"0.998"`, `"a + b"`)
   * @param Args Types of arguments for the resulting function
   * @param transform Function providing mapping from Args
   *
   * The `Args` must be default-constructible types, since transform need to be
   * called at compilation time to determine the list of available parameters.
   */
  template <class... Args>
  std::function<double(Args...)>
  compile(const std::string& expr,
          std::function<std::unordered_map<std::string, double>(Args...)> transform) {
    try {
      // trivial function if string is representation of float
      std::size_t pos;
      float expr_value = std::stof(expr, &pos);
      if (pos < expr.size()) {
        throw std::invalid_argument("unparsed trailing characters");
      }
      return [expr_value](Args... /* args */) { return expr_value; };
    } catch (std::exception& e) {
      // more complicated function otherwise
      std::vector<std::string> params;
      // Call transform with default values to detect parameter names
      for (auto& p : transform(Args{}...)) {
        params.push_back(p.first);
      }
      auto compiled_expr = _compile(expr, params);
      return [compiled_expr, transform](Args... args) {
        return compiled_expr(transform(std::forward<Args>(args)...));
      };
    }
  };

  /**
   * @brief Compile expression `expr` to std::function
   * @param expr String expression to compile (e.g. `"a + b"`)
   * @param params List of parameter names used in the expression (e.g. `{"a", "b"}`)
   *
   * The resulting function accepts a dictionary (`std::unordered_map`) of
   * parameter values.
   */
  std::function<double(const std::unordered_map<std::string, double>&)>
  _compile(const std::string& expr, std::vector<std::string> params);

private:
  unsigned int m_function_id = 0;
  std::mutex m_interpreter_mutex;

  ALGORITHMS_DEFINE_LOGGED_SERVICE(EvaluatorSvc);
};

} // namespace eicrecon
