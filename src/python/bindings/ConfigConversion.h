// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// Conversion between EICrecon config structs and Python values, by Boost.PFR field-name
// reflection. Handles, recursively:
//   - arithmetic types, std::string            -> int/float/bool/str (pybind11 casters)
//   - enums                                    -> int (underlying value)
//   - aggregate structs                        -> dict {field: value}
//   - std::vector, std::array, std::pair       -> list
//   - std::map<std::string, T>                 -> dict
//   - std::variant<Ts...>                      -> the alternative's value; alternatives that are
//                                                 structs are tagged {"type": name, ...}
//   - fixed-size Eigen vectors (e.g. Acts::Vector4) -> list
//   - std::function                            -> opaque eicrecon._core.CppFunction
// Conversions are exact: doubles are passed as doubles, never through strings.

#pragma once

#include <JANA/Utils/JTypeInfo.h>
#include <boost/pfr.hpp>
#include <boost/pfr/core_name.hpp>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <cxxabi.h>
#include <array>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#if __has_include(<Eigen/Core>)
#include <Eigen/Core>
#define EICRECON_PY_HAVE_EIGEN 1
#endif

namespace eicrecon::python {

namespace py = pybind11;

/// Type-erased holder of a C++ std::function config value (e.g. a track point cut computed from
/// geometry). Python can pass these around and back into configs, but not call or create them.
struct __attribute__((visibility("default"))) CppFunction {
  std::string type_name;
  std::string description;
  std::shared_ptr<void> function; // points to a std::function<...> of type type_name
};

/// Wrap a C++ callable for use as a config value of type Fn (a std::function)
template <class Fn> CppFunction make_cpp_function(Fn fn, std::string description = "set") {
  return CppFunction{JTypeInfo::demangle<Fn>(), std::move(description),
                     std::make_shared<Fn>(std::move(fn))};
}

class ConversionError : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

namespace detail {

  /// Demangled name of a mangled C++ type name (the input if it cannot be demangled)
  inline std::string demangle(const char* mangled) {
    int status = 0;
    const std::unique_ptr<char, void (*)(void*)> name{
        abi::__cxa_demangle(mangled, nullptr, nullptr, &status), std::free};
    return (status == 0 && name != nullptr) ? std::string{name.get()} : std::string{mangled};
  }

  template <class T> struct is_vector : std::false_type {};
  template <class T, class A> struct is_vector<std::vector<T, A>> : std::true_type {};
  template <class T> struct is_array : std::false_type {};
  template <class T, std::size_t N> struct is_array<std::array<T, N>> : std::true_type {};
  template <class T> struct is_pair : std::false_type {};
  template <class A, class B> struct is_pair<std::pair<A, B>> : std::true_type {};
  template <class T> struct is_string_map : std::false_type {};
  template <class V, class C, class A>
  struct is_string_map<std::map<std::string, V, C, A>> : std::true_type {};
  template <class T> struct is_variant : std::false_type {};
  template <class... Ts> struct is_variant<std::variant<Ts...>> : std::true_type {};
  template <class T> struct is_function : std::false_type {};
  template <class R, class... As> struct is_function<std::function<R(As...)>> : std::true_type {};

#ifdef EICRECON_PY_HAVE_EIGEN
  template <class T> struct is_fixed_eigen : std::false_type {};
  template <class S, int R, int C, int O, int MR, int MC>
  struct is_fixed_eigen<Eigen::Matrix<S, R, C, O, MR, MC>> : std::bool_constant<(R > 0 && C > 0)> {
  };
#endif

  template <class T>
  constexpr bool is_struct_v =
      std::is_class_v<T> && std::is_aggregate_v<T> && !is_vector<T>::value && !is_array<T>::value &&
      !is_pair<T>::value && !std::is_same_v<T, std::string>;

  template <class V> struct variant_tags_impl;
  template <class... Ts> struct variant_tags_impl<std::variant<Ts...>> {
    static auto get() { return std::tuple<std::type_identity<Ts>...>{}; }
  };
  template <class V> auto variant_tags() { return variant_tags_impl<V>::get(); }

  template <class T> std::string short_name() {
    auto name = JTypeInfo::demangle<T>();
    auto pos  = name.rfind("::");
    return pos == std::string::npos ? name : name.substr(pos + 2);
  }

} // namespace detail

template <class T> py::object to_py(const T& value);
template <class T> T from_py(py::handle obj, const std::string& path);

template <class T> py::dict struct_to_dict(const T& value) {
  py::dict d;
  boost::pfr::for_each_field_with_name(value, [&](std::string_view name, const auto& field) {
    d[py::str(std::string(name))] = to_py(field);
  });
  return d;
}

/// Overlay dict entries onto `value`; unknown keys raise ConversionError
template <class T>
void struct_update(T& value, py::handle obj, const std::string& path, bool allow_type_key) {
  if (!py::isinstance<py::dict>(obj)) {
    throw ConversionError(path + ": expected a dict for " + JTypeInfo::demangle<T>());
  }
  auto d = py::reinterpret_borrow<py::dict>(obj);
  std::set<std::string> used;
  boost::pfr::for_each_field_with_name(value, [&](std::string_view name_sv, auto& field) {
    std::string name(name_sv);
    if (d.contains(name)) {
      using FieldT = std::decay_t<decltype(field)>;
      field        = from_py<FieldT>(d[py::str(name)], path.empty() ? name : path + "." + name);
      used.insert(name);
    }
  });
  for (auto item : d) {
    auto key = py::cast<std::string>(py::str(item.first));
    if (allow_type_key && key == "type") {
      continue;
    }
    if (used.count(key) == 0) {
      throw ConversionError((path.empty() ? "" : path + ": ") + "unknown config field '" + key +
                            "' for " + JTypeInfo::demangle<T>());
    }
  }
}

template <class T> py::object to_py(const T& value) {
  using namespace detail;
  if constexpr (std::is_enum_v<T>) {
    return py::int_(static_cast<std::underlying_type_t<T>>(value));
  } else if constexpr (std::is_arithmetic_v<T> || std::is_same_v<T, std::string>) {
    return py::cast(value);
  } else if constexpr (is_vector<T>::value || is_array<T>::value) {
    py::list l;
    for (const auto& v : value) {
      l.append(to_py(v));
    }
    return std::move(l);
  } else if constexpr (is_pair<T>::value) {
    py::list l;
    l.append(to_py(value.first));
    l.append(to_py(value.second));
    return std::move(l);
  } else if constexpr (is_string_map<T>::value) {
    py::dict d;
    for (const auto& [k, v] : value) {
      d[py::str(k)] = to_py(v);
    }
    return std::move(d);
  } else if constexpr (is_variant<T>::value) {
    return std::visit(
        [](const auto& alt) -> py::object {
          using A = std::decay_t<decltype(alt)>;
          if constexpr (is_struct_v<A>) {
            py::dict d = struct_to_dict(alt);
            py::dict out;
            out["type"] = short_name<A>();
            for (auto item : d) {
              out[item.first] = item.second;
            }
            return std::move(out);
          } else {
            return to_py(alt);
          }
        },
        value);
  } else if constexpr (is_function<T>::value) {
    // std::function values cannot be inspected: report the type of the stored callable (e.g. a
    // named functor or a lambda), which distinguishes callables but not their state
    CppFunction f{JTypeInfo::demangle<T>(),
                  value ? detail::demangle(value.target_type().name()) : std::string{"empty"},
                  std::make_shared<T>(value)};
    return py::cast(std::move(f));
#ifdef EICRECON_PY_HAVE_EIGEN
  } else if constexpr (is_fixed_eigen<T>::value) {
    py::list l;
    for (Eigen::Index i = 0; i < value.size(); ++i) {
      l.append(py::float_(static_cast<double>(value(i))));
    }
    return std::move(l);
#endif
  } else if constexpr (is_struct_v<T>) {
    return struct_to_dict(value);
  } else {
    static_assert(sizeof(T) == 0, "eicrecon python: unsupported config field type");
  }
}

namespace detail {
  template <class V, std::size_t I = 0>
  bool variant_from_tagged(V& out, const std::string& tag, py::handle obj,
                           const std::string& path) {
    if constexpr (I < std::variant_size_v<V>) {
      using A = std::variant_alternative_t<I, V>;
      if constexpr (is_struct_v<A>) {
        if (short_name<A>() == tag) {
          A alt{};
          struct_update(alt, obj, path, true);
          out = std::move(alt);
          return true;
        }
      }
      return variant_from_tagged<V, I + 1>(out, tag, obj, path);
    }
    return false;
  }
} // namespace detail

template <class T> T from_py(py::handle obj, const std::string& path) {
  using namespace detail;
  auto fail = [&](const std::string& what) -> ConversionError {
    return ConversionError(path + ": cannot convert " +
                           std::string(py::str(py::type::of(obj).attr("__name__"))) + " to " +
                           JTypeInfo::demangle<T>() + (what.empty() ? "" : " (" + what + ")"));
  };
  if constexpr (std::is_enum_v<T>) {
    if (!py::isinstance<py::int_>(obj)) {
      throw fail("enums are given as integers");
    }
    return static_cast<T>(py::cast<std::underlying_type_t<T>>(obj));
  } else if constexpr (std::is_same_v<T, bool>) {
    if (!py::isinstance<py::bool_>(obj)) {
      throw fail("");
    }
    return py::cast<bool>(obj);
  } else if constexpr (std::is_integral_v<T>) {
    if (!py::isinstance<py::int_>(obj) || py::isinstance<py::bool_>(obj)) {
      throw fail("");
    }
    try {
      return py::cast<T>(obj);
    } catch (const py::cast_error&) {
      throw fail("out of range");
    }
  } else if constexpr (std::is_floating_point_v<T>) {
    if (!(py::isinstance<py::float_>(obj) || py::isinstance<py::int_>(obj)) ||
        py::isinstance<py::bool_>(obj)) {
      throw fail("");
    }
    return py::cast<T>(obj);
  } else if constexpr (std::is_same_v<T, std::string>) {
    if (!py::isinstance<py::str>(obj)) {
      throw fail("");
    }
    return py::cast<std::string>(obj);
  } else if constexpr (is_vector<T>::value) {
    if (!py::isinstance<py::list>(obj) && !py::isinstance<py::tuple>(obj)) {
      throw fail("expected a list");
    }
    T out;
    std::size_t i = 0;
    for (auto item : obj) {
      out.push_back(from_py<typename T::value_type>(item, path + "[" + std::to_string(i++) + "]"));
    }
    return out;
  } else if constexpr (is_array<T>::value) {
    auto seq = py::reinterpret_borrow<py::sequence>(obj);
    if (!py::isinstance<py::sequence>(obj) || py::len(seq) != std::tuple_size_v<T>) {
      throw fail("expected a list of " + std::to_string(std::tuple_size_v<T>));
    }
    T out{};
    for (std::size_t i = 0; i < out.size(); ++i) {
      out[i] = from_py<typename T::value_type>(seq[i], path + "[" + std::to_string(i) + "]");
    }
    return out;
  } else if constexpr (is_pair<T>::value) {
    auto seq = py::reinterpret_borrow<py::sequence>(obj);
    if (!py::isinstance<py::sequence>(obj) || py::len(seq) != 2) {
      throw fail("expected a list of 2");
    }
    return T{from_py<typename T::first_type>(seq[0], path + "[0]"),
             from_py<typename T::second_type>(seq[1], path + "[1]")};
  } else if constexpr (is_string_map<T>::value) {
    if (!py::isinstance<py::dict>(obj)) {
      throw fail("expected a dict");
    }
    T out;
    for (auto item : py::reinterpret_borrow<py::dict>(obj)) {
      auto key = py::cast<std::string>(item.first);
      out.emplace(key, from_py<typename T::mapped_type>(item.second, path + "[" + key + "]"));
    }
    return out;
  } else if constexpr (is_variant<T>::value) {
    if (py::isinstance<py::dict>(obj) && obj.contains("type")) {
      T out;
      auto tag = py::cast<std::string>(obj["type"]);
      if (variant_from_tagged(out, tag, obj, path)) {
        return out;
      }
      throw fail("no alternative named '" + tag + "'");
    }
    // untagged: first alternative that accepts the value
    std::optional<T> out;
    std::apply(
        [&](auto... tags) {
          (([&] {
             using A = typename decltype(tags)::type;
             if (out || is_struct_v<A>) {
               return;
             }
             try {
               out = from_py<A>(obj, path);
             } catch (const ConversionError&) {
             }
           }()),
           ...);
        },
        variant_tags<T>());
    if (!out) {
      throw fail("");
    }
    return *out;
  } else if constexpr (is_function<T>::value) {
    if (!py::isinstance<CppFunction>(obj)) {
      throw fail("std::function values can only come from C++ (CppFunction)");
    }
    const auto& f = py::cast<const CppFunction&>(obj);
    if (f.type_name != JTypeInfo::demangle<T>()) {
      throw fail("CppFunction holds " + f.type_name);
    }
    return *std::static_pointer_cast<T>(f.function);
#ifdef EICRECON_PY_HAVE_EIGEN
  } else if constexpr (is_fixed_eigen<T>::value) {
    auto seq = py::reinterpret_borrow<py::sequence>(obj);
    T out;
    if (!py::isinstance<py::sequence>(obj) ||
        py::len(seq) != static_cast<std::size_t>(out.size())) {
      throw fail("expected a list of " + std::to_string(out.size()));
    }
    for (Eigen::Index i = 0; i < out.size(); ++i) {
      out(i) = from_py<typename T::Scalar>(seq[i], path + "[" + std::to_string(i) + "]");
    }
    return out;
#endif
  } else if constexpr (is_struct_v<T>) {
    T out{};
    struct_update(out, obj, path, false);
    return out;
  } else {
    static_assert(sizeof(T) == 0, "eicrecon python: unsupported config field type");
  }
}

} // namespace eicrecon::python
