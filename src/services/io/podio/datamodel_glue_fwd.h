// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 Wouter Deconinck
//
// Forward declarations for PODIO datamodel glue
//
// **IMPORTANT FOR DEVELOPERS:**
// This lightweight header provides only PodioTypeMap type traits.
// Use this when you only need type information (e.g., in factories using PodioInput/PodioOutput).
//
// If you need the full visitor pattern (VisitPodioCollection), include datamodel_glue.h instead.
// That header includes all PODIO umbrella headers and should only be used where absolutely necessary.
//
// **Build Time Impact:**
// - Including this header: ~50ms compile time (minimal)
// - Including datamodel_glue.h: ~6000ms compile time (all PODIO types)
//
// JOmniFactory.h includes this file, so all factories get type traits without umbrella headers.

#pragma once

// PodioTypeMap provides type traits for podio types
// This mirrors the structure written by the legacy python generator,
// and puts the types in the format expected by JANA2.
template <typename T> struct PodioTypeMap {
  using collection_t = typename T::collection_type;
  using mutable_t    = typename T::mutable_type;
};
