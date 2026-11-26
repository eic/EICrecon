// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// Named accessors of podio object IDs, for use as template arguments of FilterMatching_factory.
//
// A lambda written inline in a template argument has a closure type unique to its translation
// unit, so the resulting factory type cannot be named anywhere else (e.g. in the Python
// bindings, src/python). A captureless lambda held by an inline constexpr variable has a single
// closure type in all translation units.

#pragma once

namespace eicrecon::object_id {

/// obj->getObjectID()
inline constexpr auto self = [](auto* obj) { return obj->getObjectID(); };
/// obj->getCluster().getObjectID()
inline constexpr auto cluster = [](auto* obj) { return obj->getCluster().getObjectID(); };
/// obj->getRec().getObjectID()
inline constexpr auto rec = [](auto* obj) { return obj->getRec().getObjectID(); };
/// obj->getSim().getObjectID()
inline constexpr auto sim = [](auto* obj) { return obj->getSim().getObjectID(); };
/// obj->getTo().getObjectID()
inline constexpr auto to = [](auto* obj) { return obj->getTo().getObjectID(); };

} // namespace eicrecon::object_id
