// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Simon Gardner

#pragma once

namespace eicrecon {

template <class T> struct SubDivideCollectionConfig {
  std::function<std::vector<size_t>(const T&)> function;
};

} // namespace eicrecon
