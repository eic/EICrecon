// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck

#pragma once

namespace eicrecon {

// This array relates the Acts and EDM4eic covariance matrices, including
// the unit conversion to get from Acts units into EDM4eic units.
//
// Note: std::map is not constexpr, so we use a constexpr std::array
// std::array initialization need double braces since arrays are aggregates
// ref: https://en.cppreference.com/w/cpp/language/aggregate_initialization
static constexpr std::array<std::pair<Acts::BoundIndices, double>, 6> edm4eic_indexed_units{
    {{Acts::eBoundLoc0, Acts::UnitConstants::mm},
     {Acts::eBoundLoc1, Acts::UnitConstants::mm},
     {Acts::eBoundPhi, 1.},
     {Acts::eBoundTheta, 1.},
     {Acts::eBoundQOverP, 1. / Acts::UnitConstants::GeV},
     {Acts::eBoundTime, Acts::UnitConstants::ns}}};

} // namespace eicrecon
