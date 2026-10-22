// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Derek Anderson

#pragma once

namespace eicrecon {

template <class SourceT, class SourceAssocT, class AssocT, class TargetT = SourceT,
          class TargetAssocT = SourceAssocT>
struct CopyAssociationsConfig {

  /// rule to get source from source association
  std::function<SourceT(const SourceAssocT&)> getSourceFrom;

  /// rule to get object associated to source from source association
  std::function<AssocT(const SourceAssocT&)> getSourceTo;

  /// rule to set target in target association
  std::function<void(const TargetT&, typename TargetAssocT::mutable_type&)> setTargetFrom;

  /// rule to set object associated to target in target association
  std::function<void(const AssocT&, typename TargetAssocT::mutable_type&)> setTargetTo;
};

} // namespace eicrecon
