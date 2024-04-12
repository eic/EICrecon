// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Simon Gardner

#pragma once

namespace eicrecon {

template <class T> struct SubDivideCollectionConfig {
  std::function<std::vector<int>(const T&)> function;
};

} // namespace eicrecon
