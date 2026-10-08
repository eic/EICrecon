// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Derek Anderson

#pragma once

namespace eicrecon {

template <class TSource, class TTarget, class TLinked, class TSourceAssoc,
          class TTargetAssoc = TSourceAssoc>
struct CopyAssociationsConfig {

  /// TODO punch up:
  /// rule to get source from source association
  std::function<TSource(const TSourceAssoc&)> getSourceFrom;

  /// TODO punch up:
  /// rule to get link from source association
  std::function<TLinked(const TSourceAssoc&)> getSourceTo;

  /// TODO punch up:
  /// rule to set target in target association
  std::function<void(const TTarget&, TTargetAssoc&)> setTargetFrom;

  /// TODO punch up:
  /// rule to set link in target association
  std::function<void(const TLinked&, TTargetAssoc&)> setTargetTo;
};

} // namespace eicrecon
