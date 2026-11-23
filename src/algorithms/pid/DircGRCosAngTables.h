// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 hpDIRC GR effort

#pragma once

namespace eicrecon::dircgr {

inline constexpr int kNCosAng      = 4562;
inline constexpr int kCosAngOffset = 2730; // index = round(z[mm]) + offset

extern const double kCosAngUpper[kNCosAng];
extern const double kCosAngLower[kNCosAng];

} // namespace eicrecon::dircgr
