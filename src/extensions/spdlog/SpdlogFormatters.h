#pragma once

// IWYU pragma: always_keep
// since IWYU otherwise removes this headers from output that need it
// FIXME: this is only valid as of iwyu v21; until then use keep on includes

#include <system_error>
#include <type_traits>

#include <fmt/core.h>
#include <fmt/ostream.h>

#include <edm4eic/Cov2f.h>
#include <edm4eic/Cov3f.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/Vector3d.h>

template <> struct fmt::formatter<edm4eic::Cov2f> : fmt::ostream_formatter {};
template <> struct fmt::formatter<edm4eic::Cov3f> : fmt::ostream_formatter {};
template <> struct fmt::formatter<edm4hep::Vector3f> : fmt::ostream_formatter {};
template <> struct fmt::formatter<edm4hep::Vector3d> : fmt::ostream_formatter {};

template <> struct fmt::formatter<std::error_code> : fmt::ostream_formatter {};
