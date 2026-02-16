// Copyright 2024, Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//
#pragma once

#include <algorithms/geo.h>
#include <functional>
#include <type_traits>
#include <utility>

namespace eicrecon {

// ----------------------------------------------------------------------------
// Chain wrapper type for explicit member function call chaining
// Usage: RangeSplit<Chain<&A::getB, &B::getC>> splits on A->getB()->getC()
// ----------------------------------------------------------------------------
template <auto... MemberFunctionPtrs> struct ChainTag {};

template <auto... MemberFunctionPtrs> inline constexpr ChainTag<MemberFunctionPtrs...> Chain{};

// ----------------------------------------------------------------------------
// Helper to invoke a chain of member function calls
// Usage: ChainInvoker<&A::getB, &B::getC>::invoke(a) chains A->getB()->getC()
// ----------------------------------------------------------------------------
template <auto... MemberFunctionPtrs> struct ChainInvoker;

// Base case: single member function
template <auto MemberFunctionPtr> struct ChainInvoker<MemberFunctionPtr> {
  template <typename T> static auto invoke(T&& instance) {
    return std::invoke(MemberFunctionPtr, std::forward<T>(instance));
  }
};

// Recursive case: chain multiple member functions
template <auto FirstMemberFunctionPtr, auto... RestMemberFunctionPtrs>
struct ChainInvoker<FirstMemberFunctionPtr, RestMemberFunctionPtrs...> {
  template <typename T> static auto invoke(T&& instance) {
    auto&& nested = std::invoke(FirstMemberFunctionPtr, std::forward<T>(instance));
    return ChainInvoker<RestMemberFunctionPtrs...>::invoke(nested);
  }
};

// ----------------------------------------------------------------------------
// Helper to detect ChainTag at compile time
template <typename T> struct is_chain : std::false_type {};
template <auto... MemberFunctionPtrs>
struct is_chain<ChainTag<MemberFunctionPtrs...>> : std::true_type {};

// Helper to invoke either a direct member function pointer or a Chain value
// ----------------------------------------------------------------------------
template <auto Accessor> struct AccessorInvoker {
  template <typename T> static auto invoke(T&& instance) {
    if constexpr (is_chain<decltype(Accessor)>::value) {
      // Accessor is a Chain variable, extract the tag and invoke
      return invoke_chain(std::forward<T>(instance), Accessor);
    } else {
      // Accessor is a member function pointer
      static_assert(std::is_member_function_pointer_v<decltype(Accessor)>,
                    "Accessor must be a member function pointer or chain");
      return std::invoke(Accessor, std::forward<T>(instance));
    }
  }

private:
  template <typename T, auto... MemberFunctionPtrs>
  static auto invoke_chain(T&& instance, ChainTag<MemberFunctionPtrs...>) {
    return ChainInvoker<MemberFunctionPtrs...>::invoke(std::forward<T>(instance));
  }
};

// ----------------------------------------------------------------------------
// Functor to split collection based on a range of values
// ----------------------------------------------------------------------------
template <auto Accessor> class RangeSplit {
public:
  RangeSplit(const std::vector<std::pair<double, double>> ranges) : m_ranges(ranges) {};

  template <typename T> std::vector<size_t> operator()(T&& instance) const {
    std::vector<size_t> ids;
    auto value = AccessorInvoker<Accessor>::invoke(instance);
    // Check if requested value is within the ranges
    for (std::size_t i = 0; i < m_ranges.size(); i++) {
      if (value > m_ranges[i].first && value < m_ranges[i].second) {
        ids.push_back(i);
      }
    }
    return ids;
  }

private:
  const std::vector<std::pair<double, double>> m_ranges;
};

// ----------------------------------------------------------------------------
// Functor to split collection based on geometry
// ----------------------------------------------------------------------------
class GeometrySplit {
public:
  GeometrySplit(const std::vector<std::vector<long int>> ids, const std::string readout,
                const std::vector<std::string> divisions)
      : m_ids(ids)
      , m_divisions(divisions)
      , m_readout(readout)
      , is_init(std::make_shared<std::once_flag>())
      , m_id_dec(std::make_shared<dd4hep::DDSegmentation::BitFieldCoder*>())
      , m_div_ids(std::make_shared<std::vector<std::size_t>>()) {};

  template <typename T> std::vector<size_t> operator()(const T& instance) const {

    // Initialize the decoder and division ids on the first function call
    std::call_once(*is_init, &GeometrySplit::init, this);

    //Check which detector division to put the hit into
    auto cellID = instance.getCellID();
    std::vector<long int> det_ids;
    for (auto d : *m_div_ids) {
      det_ids.push_back((*m_id_dec)->get(cellID, d));
    }

    auto index = std::find(m_ids.begin(), m_ids.end(), det_ids);

    std::vector<size_t> ids;
    if (index != m_ids.end()) {
      ids.push_back(std::distance(m_ids.begin(), index));
    }
    return ids;
  }

private:
  void init() const {
    *m_id_dec = algorithms::GeoSvc::instance().detector()->readout(m_readout).idSpec().decoder();
    for (auto d : m_divisions) {
      m_div_ids->push_back((*m_id_dec)->index(d));
    }
  }

  const std::vector<std::vector<long int>> m_ids;
  const std::vector<std::string> m_divisions;
  const std::string m_readout;

  std::shared_ptr<std::once_flag> is_init;
  const std::shared_ptr<dd4hep::DDSegmentation::BitFieldCoder*> m_id_dec;
  std::shared_ptr<std::vector<size_t>> m_div_ids;
};

// ----------------------------------------------------------------------------
// Functor to split collection based on any number of collection values
// ----------------------------------------------------------------------------
template <auto... Accessors> class ValueSplit {
public:
  ValueSplit(const std::vector<std::vector<int>> ids) : m_ids(ids) {};

  template <typename T> std::vector<size_t> operator()(T&& instance) const {
    std::vector<size_t> ids;
    // Check if requested value matches any configuration combinations
    std::vector<int> values;
    (values.push_back(AccessorInvoker<Accessors>::invoke(instance)), ...);
    auto index = std::find(m_ids.begin(), m_ids.end(), values);
    if (index != m_ids.end()) {
      ids.push_back(std::distance(m_ids.begin(), index));
    }
    return ids;
  }

private:
  const std::vector<std::vector<int>> m_ids;
};

} // namespace eicrecon
