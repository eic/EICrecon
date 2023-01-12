#pragma once

#include <system_error>
#include <type_traits>

#include <fmt/core.h>
#include <fmt/ostream.h>

#include <edm4eic/Cov2f.h>
#include <edm4eic/Cov3f.h>
#include <edm4hep/Vector3f.h>
#include <edm4hep/Vector3d.h>

#include <Eigen/Core>

#if FMT_VERSION >= 90000

/*

template <typename T, typename = void>
struct has_output_operator : std::false_type {};

template <typename T>
struct has_output_operator<T, std::void_t<decltype(std::cout << std::declval<T>())>> : std::true_type {};

template <typename T, typename Char, typename Context>
struct fmt::formatter<
    T,
    std::enable_if_t<
        !fmt::has_formatter<T, Context>::value &&
        has_output_operator<T>::value,
        Char
    >
> : fmt::ostream_formatter {};

*/

// handle Eigen CRTP
template <typename T>
struct fmt::formatter<
    T,
    std::enable_if_t<
        std::is_base_of_v<Eigen::MatrixBase<T>, T>,
        char
    >
> : fmt::ostream_formatter {};

template<> struct fmt::formatter<Acts::GeometryIdentifier> : fmt::ostream_formatter {};

template<> struct fmt::formatter<edm4eic::Cov2f> : fmt::ostream_formatter {};
template<> struct fmt::formatter<edm4eic::Cov3f> : fmt::ostream_formatter {};
template<> struct fmt::formatter<edm4hep::Vector3f> : fmt::ostream_formatter {};
template<> struct fmt::formatter<edm4hep::Vector3d> : fmt::ostream_formatter {};

template<> struct fmt::formatter<std::error_code> : fmt::ostream_formatter {};

#endif // FMT_VERSION >= 90000
