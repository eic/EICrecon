// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Dmitry Kalinkin

#include <TInterpreter.h>
#include <TInterpreterValue.h>
#include <fmt/core.h>
#include <memory>
#include <sstream>
#include <stdexcept>

#include "EvaluatorSvc.h"

namespace eicrecon {

void EvaluatorSvc::init() {
  // This is needed to bypass condition in algorithms::LoggerMixin::report and
  // forward all messages to our instance of LogSvc/spdlog.
  level(algorithms::LogLevel::kTrace);
}

std::function<double(const std::unordered_map<std::string, double>&)>
EvaluatorSvc::_compile(const std::string& expr, std::vector<std::string> params) {
  std::lock_guard<std::mutex> guard(m_interpreter_mutex);

  std::string func_name = fmt::format("_eicrecon_{}", m_function_id++);
  std::ostringstream sstr;
  sstr << "double " << func_name << "(double params[]){";
  for (unsigned int param_ix = 0; const auto& p : params) {
    sstr << "double " << p << " = params[" << (param_ix++) << "];";
  }
  sstr << "return " << expr << ";";
  sstr << "}";

  TInterpreter* interp = TInterpreter::Instance();
  debug("Compiling {}", sstr.str());
  TInterpreter::EErrorCode ec = TInterpreter::kNoError;
  interp->ProcessLine(sstr.str().c_str(), &ec);
  if (ec != TInterpreter::kNoError) {
    throw std::runtime_error(fmt::format("EvaluatorSvc: failed to compile expression: {}", expr));
  }
  // Use the explicit address-of operator (&) so that TInterpreterValue stores the
  // result with pointer type (double(*)(double[])) rather than function type
  // (double(double[])). For pointer types, GetAsPointer() returns the stored
  // pointer value directly from the storage union, which is the function address
  // and remains valid after func_val is destroyed. For function types, the
  // storage holds a reference to a temporary evaluation buffer inside func_val,
  // which becomes a dangling pointer once func_val goes out of scope.
  std::unique_ptr<TInterpreterValue> func_val{gInterpreter->MakeInterpreterValue()};
  interp->Evaluate(fmt::format("&{}", func_name).c_str(), *func_val);
  typedef double (*func_t)(double params[]);
  // NOLINTNEXTLINE(performance-no-int-to-ptr)
  func_t func = (func_t)(func_val->GetAsPointer());
  if (!func) {
    throw std::runtime_error(
        fmt::format("EvaluatorSvc: failed to get function pointer for expression: {}", expr));
  }

  return [params, func](const std::unordered_map<std::string, double>& param_values) {
    std::vector<double> value_list;
    value_list.reserve(params.size());
    for (const auto& p : params) {
      value_list.push_back(param_values.at(p));
    }
    return func(value_list.data());
  };
}

} // namespace eicrecon
