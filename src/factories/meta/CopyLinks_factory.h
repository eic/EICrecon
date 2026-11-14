// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Derek Anderson

#pragma once

#include "algorithms/interfaces/WithPodConfig.h"
#include "algorithms/meta/CopyLinks.h"
#include "extensions/jana/JOmniAlgorithmFactory.h"

namespace eicrecon {

template <class SourceT, class SourceLinkT, class TargetT = SourceT,
          class TargetLinkT = SourceLinkT>
class CopyLinks_factory
    : public JOmniAlgorithmFactory<CopyLinks_factory<SourceT, SourceLinkT, TargetT, TargetLinkT>,
                                   CopyLinks<SourceT, SourceLinkT, TargetT, TargetLinkT>> {};

} // namespace eicrecon
