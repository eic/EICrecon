// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck

#pragma once

// Generation of JOmniFactory input/output handles from an algorithms::Input<...> or
// algorithms::Output<...> signature. See JOmniAlgorithmFactory.h for the slot mapping.

#include <JANA/Utils/JTypeInfo.h>
#include <algorithms/algorithm.h>
#include <algorithms/type_traits.h>
#include <gsl/pointers>
#include <podio/CollectionBase.h>

#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

namespace eicrecon::detail {

template <class C>
constexpr bool is_podio_collection_v =
    std::is_base_of_v<podio::CollectionBase, std::remove_cv_t<C>>;

/// Selects the JOmniFactory handle for one algorithm slot, and converts its content
/// into the type the algorithm's process() expects for that slot.
///
/// OptionalVariadic: for std::vector<C> input slots, accept missing collections and drop
/// them before calling process(). (algorithms >= 2.1 can express this in the algorithm
/// signature as std::vector<std::optional<C>>, but algorithms 2.0 cannot.)
template <class OmniT, class Slot, bool IsInput, bool OptionalVariadic = false> struct SlotHandle;

// ---- inputs ----

template <class OmniT, class D, bool Podio, bool Variadic, bool Optional> struct InputHandleSelect;
template <class OmniT, class D, bool Optional>
struct InputHandleSelect<OmniT, D, true, false, Optional> {
  using type = typename OmniT::template PodioInput<typename D::value_type, Optional>;
};
template <class OmniT, class D, bool Optional>
struct InputHandleSelect<OmniT, D, true, true, Optional> {
  using type = typename OmniT::template VariadicPodioInput<typename D::value_type, Optional>;
};
template <class OmniT, class D, bool Optional>
struct InputHandleSelect<OmniT, D, false, false, Optional> {
  using type = typename OmniT::template Input<D, Optional>;
};

template <class OmniT, class Slot, bool OptionalVariadic>
struct SlotHandle<OmniT, Slot, true, OptionalVariadic> {
  using data_t                    = std::remove_cv_t<algorithms::data_type_t<Slot>>;
  static constexpr bool kVariadic = algorithms::is_vector_v<Slot>;
  static constexpr bool kOptional =
      algorithms::is_optional_v<Slot> || (kVariadic && OptionalVariadic);
  static constexpr bool kPodio = is_podio_collection_v<data_t>;
  using value_t                = algorithms::input_type_t<Slot>;

  static_assert(kPodio || !kVariadic,
                "JOmniAlgorithmFactory: variadic inputs of non-podio types are not supported");
  static_assert(!algorithms::is_optional_v<data_t>,
                "JOmniAlgorithmFactory: std::vector<std::optional<C>> slots are not supported");

  using type = typename InputHandleSelect<OmniT, data_t, kPodio, kVariadic, kOptional>::type;

  static value_t get(type& h) {
    if constexpr (kPodio && kVariadic) {
      std::remove_const_t<value_t> ret;
      for (const auto* coll : h()) {
        if (kOptional && coll == nullptr) {
          continue; // missing optional collection
        }
        ret.emplace_back(coll);
      }
      return ret;
    } else if constexpr (kPodio) {
      return h();
    } else {
      const auto& objs = h();
      if (objs.empty() || objs.front() == nullptr) {
        if constexpr (kOptional) {
          return nullptr;
        } else {
          throw std::runtime_error("JOmniAlgorithmFactory: missing required input of type " +
                                   JTypeInfo::demangle<data_t>());
        }
      } else {
        return objs.front();
      }
    }
  }
};

// ---- outputs ----

template <class OmniT, class D, bool Podio, bool Variadic> struct OutputHandleSelect;
template <class OmniT, class D> struct OutputHandleSelect<OmniT, D, true, false> {
  using type = typename OmniT::template PodioOutput<typename D::value_type>;
};
template <class OmniT, class D> struct OutputHandleSelect<OmniT, D, true, true> {
  using type = typename OmniT::template VariadicPodioOutput<typename D::value_type>;
};
template <class OmniT, class D> struct OutputHandleSelect<OmniT, D, false, false> {
  using type = typename OmniT::template Output<std::remove_pointer_t<D>>;
};

template <class OmniT, class Slot, bool OptionalVariadic>
struct SlotHandle<OmniT, Slot, false, OptionalVariadic> {
  using data_t                    = algorithms::data_type_t<Slot>;
  static constexpr bool kVariadic = algorithms::is_vector_v<Slot>;
  static constexpr bool kPodio    = is_podio_collection_v<data_t>;
  using value_t                   = algorithms::output_type_t<Slot>;

  static_assert(kPodio || (std::is_pointer_v<data_t> && !kVariadic),
                "JOmniAlgorithmFactory: non-podio outputs must be declared as T* (non-variadic)");

  using type = typename OutputHandleSelect<OmniT, data_t, kPodio, kVariadic>::type;

  static value_t get(type& h) {
    if constexpr (kPodio && kVariadic) {
      std::remove_const_t<value_t> ret;
      for (auto& coll : h()) {
        ret.emplace_back(coll.get());
      }
      return ret;
    } else if constexpr (kPodio) {
      return h().get();
    } else {
      // The algorithm receives the address of a (null) pointer slot and fills it
      return &h().emplace_back();
    }
  }
};

/// Handles for all slots of an algorithms::Input<...> or algorithms::Output<...>
template <class OmniT, class DataTuple, bool IsInput, bool OptionalVariadic = false>
class SlotHandles;

template <class OmniT, class... Slots, bool IsInput, bool OptionalVariadic>
class SlotHandles<OmniT, std::tuple<Slots...>, IsInput, OptionalVariadic> {
  template <class Slot> using handle_t = SlotHandle<OmniT, Slot, IsInput, OptionalVariadic>;
  using value_t = std::conditional_t<IsInput, typename algorithms::Input<Slots...>::value_type,
                                     typename algorithms::Output<Slots...>::value_type>;

public:
  // Handles self-register with the owner in their constructor. Since std::tuple element
  // construction order is unspecified (reverse in libstdc++), call reorder() afterwards.
  explicit SlotHandles(OmniT* owner) : m_handles(((void)sizeof(Slots*), owner)...) {}

  /// Move this set's handles to the end of the owner's registry, in slot order, so that
  /// positional InputTags/OutputTags follow the algorithm signature.
  template <class Base> void reorder(std::vector<Base*>& registered) {
    std::apply(
        [&registered](auto&... h) {
          (std::erase(registered, static_cast<Base*>(&h)), ...);
          (registered.push_back(static_cast<Base*>(&h)), ...);
        },
        m_handles);
  }

  value_t get() {
    return std::apply([](auto&... h) { return value_t{handle_t<Slots>::get(h)...}; }, m_handles);
  }

private:
  std::tuple<typename handle_t<Slots>::type...> m_handles;
};

} // namespace eicrecon::detail
