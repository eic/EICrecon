// Copyright (C) 2026 Wouter Deconinck
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

namespace eicrecon {

// A helper class to combine multiple lambdas into a single callable object.
//
//  multilambda _toDouble = {
//      [](const std::string& v) { return dd4hep::_toDouble(v); },
//      [](const double& v) { return v; },
//  };
//
template <typename... L> struct multilambda : L... {
  using L::operator()...;
  constexpr multilambda(L... lambda) : L(std::move(lambda))... {}
};

} // namespace eicrecon
