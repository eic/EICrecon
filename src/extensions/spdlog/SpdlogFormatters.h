#pragma once

#include <system_error>
#include <type_traits>

#include <fmt/core.h>
#include <fmt/ostream.h>

#include <edm4eic/Cov2f.h>
#include <edm4eic/Cov3f.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/Vector3d.h>

#if FMT_VERSION >= 90000

template<> struct fmt::formatter<edm4eic::Cov2f> : fmt::ostream_formatter {};
template<> struct fmt::formatter<edm4eic::Cov3f> : fmt::ostream_formatter {};
template<> struct fmt::formatter<edm4hep::Vector3f> : fmt::ostream_formatter {};
template<> struct fmt::formatter<edm4hep::Vector3d> : fmt::ostream_formatter {};

template<> struct fmt::formatter<std::error_code> : fmt::ostream_formatter {};

#endif // FMT_VERSION >= 90000
