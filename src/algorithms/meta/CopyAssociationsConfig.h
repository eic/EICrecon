// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Derek Anderson

#pragma once

namespace eicrecon {

template <class SourceT, class SourceAssocT, class AssocT, class TargetT = SourceT,
          class TargetAssocT = SourceAssocT>
struct CopyAssociationsConfig {

  /// TODO punch up:
  /// rule to get source from source association
  std::function<SourceT(const SourceAssocT&)> getSourceFrom;

  /// TODO punch up:
  /// rule to get link from source association
  std::function<AssocT(const SourceAssocT&)> getSourceTo;

  /// TODO punch up:
  /// rule to set target in target association
  std::function<void(const TargetT&, typename TargetAssocT::mutable_type&)> setTargetFrom;

  /// TODO punch up:
  /// rule to set link in target association
  std::function<void(const AssocT&, typename TargetAssocT::mutable_type&)> setTargetTo;
};

} // namespace eicrecon
