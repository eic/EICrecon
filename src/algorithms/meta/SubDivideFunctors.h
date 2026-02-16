// Copyright 2024, Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//
#pragma once

#include <algorithms/geo.h>
#include <functional>
#include <vector>
#include <numeric>

namespace eicrecon {

// Helper to get the class type from a member function pointer
template <typename T> struct member_function_class;

// Specialization for non-const member functions
template <typename R, typename C> struct member_function_class<R (C::*)()> {
  using type = C;
};

// Specialization for const member functions
template <typename R, typename C> struct member_function_class<R (C::*)() const> {
  using type = C;
};

template <auto MemberFunctionPtr> struct MemberFunctionReturnType {
  using type =
      decltype((std::declval<typename member_function_class<decltype(MemberFunctionPtr)>::type>().*
                MemberFunctionPtr)());
};

template <auto... MemberFunctionPtrs> struct MemberFunctionReturnTypes {
  using type = std::tuple<typename MemberFunctionReturnType<MemberFunctionPtrs>::type...>;
};

template <auto First, auto... Rest> struct FirstMemberFunctionPtr {
  static constexpr auto value = First;
};

template <typename BooleanFunction, typename... Types, std::size_t... Is>
constexpr auto make_comparison_tuple_impl(const BooleanFunction& func, std::index_sequence<Is...>) {
  return std::make_tuple(std::function<bool(Types, Types)>{func}... // Expand for each type
  );
}

template <typename BooleanFunction, typename... Types>
constexpr auto make_comparison_tuple(const BooleanFunction& func) {
  return make_comparison_tuple_impl<BooleanFunction, Types...>(func,
                                                               std::index_sequence_for<Types...>{});
}

// ----------------------------------------------------------------------------
// Chain wrapper type for explicit member function call chaining
// Usage: Chain<&A::getB, &B::getC> chains A->getB()->getC()
// ----------------------------------------------------------------------------
template <auto... MemberFunctionPtrs> struct Chain {};

// ----------------------------------------------------------------------------
// Helper to invoke a chain of member function calls
// ----------------------------------------------------------------------------
template <auto... MemberFunctionPtrs> struct ChainInvoker;

// Base case: single member function
template <auto MemberFunctionPtr> struct ChainInvoker<MemberFunctionPtr> {
  template <typename T> static auto invoke(T& instance) { return (instance.*MemberFunctionPtr)(); }
};

// Recursive case: chain multiple member functions
template <auto FirstMemberFunctionPtr, auto... RestMemberFunctionPtrs>
struct ChainInvoker<FirstMemberFunctionPtr, RestMemberFunctionPtrs...> {
  template <typename T> static auto invoke(T& instance) {
    auto nested = (instance.*FirstMemberFunctionPtr)();
    return ChainInvoker<RestMemberFunctionPtrs...>::invoke(nested);
  }
};

// ----------------------------------------------------------------------------
// Functor to split collection based on a range of values
// ----------------------------------------------------------------------------
template <typename... Chains> class RangeSplit;

// Specialization: single Chain
template <auto... MemberFunctionPtrs> class RangeSplit<Chain<MemberFunctionPtrs...>> {
public:
  // Deduce the return type of the member function pointer
  using ValueType = typename MemberFunctionReturnType<MemberFunctionPtr>::type;
  // Assure that the ValueType is arithmetic
  static_assert(std::is_arithmetic_v<ValueType>, "RangeSplit requires an arithmetic value type");

  RangeSplit(const std::vector<std::pair<ValueType, ValueType>>& ranges, const bool inside = true)
      : m_ranges(ranges), m_inside(ranges.size(), inside) {}

  RangeSplit(const std::vector<std::pair<ValueType, ValueType>>& ranges,
             const std::vector<bool>& inside)
      : m_ranges(ranges), m_inside(inside) {
    if constexpr (inside.size() != ranges.size()) {
      throw std::invalid_argument("Size of inside must match the size of ranges");
    }
  }

  template <typename T> std::vector<size_t> operator()(T& instance) const {
    std::vector<size_t> ids;
    auto value = (instance.*MemberFunctionPtr)();
    //Check if requested value is within the ranges
    for (size_t i = 0; i < m_ranges.size(); i++) {
      if (m_inside[i]) {
        if (value >= m_ranges[i].first && value <= m_ranges[i].second) {
          ids.push_back(i);
        }
      } else {
        if (value < m_ranges[i].first || value > m_ranges[i].second) {
          ids.push_back(i);
        }
      }
    }
    return ids;
  }

private:
  const std::vector<std::pair<ValueType, ValueType>> m_ranges;
  const std::vector<bool> m_inside;
};

// ----------------------------------------------------------------------------
// Functor to split collection based on geometry
// ----------------------------------------------------------------------------
class GeometrySplit {
public:
  GeometrySplit(const std::vector<std::vector<long int>>& ids, const std::string readout,
                const std::vector<std::string>& divisions)
      : m_ids(ids)
      , m_divisions(divisions)
      , m_readout(readout)
      , is_init(std::make_shared<std::once_flag>())
      , m_id_dec(std::make_shared<dd4hep::DDSegmentation::BitFieldCoder*>())
      , m_div_ids(std::make_shared<std::vector<std::size_t>>()) {};

  template <typename T> std::vector<size_t> operator()(T& instance) const {

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
template <auto... MemberFunctionPtrs> class ValueSplit {
public:
  ValueSplit(const std::vector<std::vector<int>>& ids, const bool matching = true)
      : m_ids(ids), m_matching(matching) {};

  template <typename T> std::vector<size_t> operator()(const T& instance) const {
    std::vector<size_t> ids;
    // Check if requested value matches any configuration combinations
    std::vector<int> values;
    (values.push_back((instance.*MemberFunctionPtrs)()), ...);
    auto index = std::find(m_ids.begin(), m_ids.end(), values);
    if (index != m_ids.end()) {
      ids.push_back(std::distance(m_ids.begin(), index));
    }
    return ids;
  }

private:
  const std::vector<std::vector<int>> m_ids;
  const bool m_matching;
};

// ----------------------------------------------------------------------------
// Functor to split collection based on any number of collection values
// ----------------------------------------------------------------------------
template <auto... MemberFunctionPtrs> class BooleanSplit {
public:
  using ReturnTypes         = typename MemberFunctionReturnTypes<MemberFunctionPtrs...>::type;
  using ComparisonFunctions = std::tuple<
      std::function<bool(typename MemberFunctionReturnType<MemberFunctionPtrs>::type,
                         typename MemberFunctionReturnType<MemberFunctionPtrs>::type)>...>;

  // Get the first member function pointer from the parameter pack so that single function constructors can be used
  static constexpr auto FirstMemberFunction = FirstMemberFunctionPtr<MemberFunctionPtrs...>::value;
  using FirstReturnType    = typename MemberFunctionReturnType<FirstMemberFunction>::type;
  using ComparisonFunction = std::function<bool(FirstReturnType, FirstReturnType)>;

  // Declare the tuple size as a static constexpr member
  static constexpr std::size_t TupleSize = std::tuple_size<ReturnTypes>::value;

  // Ensure that TupleSize is at least 1
  static_assert(TupleSize > 0, "BooleanSplit requires at least one member function pointer.");

  BooleanSplit(const std::vector<
                   std::tuple<typename MemberFunctionReturnType<MemberFunctionPtrs>::type...>>& ids,
               const ComparisonFunction& comparison = std::equal_to{})
      : m_ids(ids)
      , m_comparisons(
            make_comparison_tuple<ComparisonFunction,
                                  typename MemberFunctionReturnType<MemberFunctionPtrs>::type...>(
                comparison)) {};

  BooleanSplit(
      const std::tuple<typename MemberFunctionReturnType<MemberFunctionPtrs>::type...>& ids,
      const ComparisonFunction& comparison = std::equal_to{})
      : m_ids(1, ids)
      , m_comparisons(
            make_comparison_tuple<ComparisonFunction,
                                  typename MemberFunctionReturnType<MemberFunctionPtrs>::type...>(
                comparison)) {};

  BooleanSplit(const std::vector<
                   std::tuple<typename MemberFunctionReturnType<MemberFunctionPtrs>::type...>>& ids,
               const std::vector<ComparisonFunction>& comparisons)
      : m_ids(ids), m_comparisons(comparisons) {
    if (ids.size() != comparisons.size()) {
      throw std::invalid_argument(
          "Size of values to compare must match the size of boolean functions");
    }
  }

  template <typename T> std::vector<size_t> operator()(const T& instance) const {
    std::vector<size_t> ids;
    // Check if requested value matches any configuration combinations
    ReturnTypes values{(instance.*MemberFunctionPtrs)()...};
    for (size_t i = 0; i < m_ids.size(); ++i) {
      if (compareTuples(m_ids[i], values, m_comparisons)) {
        ids.push_back(i);
      }
    }
    return ids;
  }

private:
  std::vector<std::tuple<typename MemberFunctionReturnType<MemberFunctionPtrs>::type...>> m_ids;
  ComparisonFunctions m_comparisons;

  template <typename Tuple1, typename Tuple2, typename Tuple3, std::size_t... Is>
  static bool compareTuplesImpl(const Tuple1& tuple1, const Tuple2& tuple2,
                                const Tuple3& comparisons, std::index_sequence<Is...>) {
    return (... && (std::get<Is>(comparisons)(std::get<Is>(tuple1), std::get<Is>(tuple2))));
  }

  template <typename Tuple1, typename Tuple2, typename Tuple3>
  static bool compareTuples(const Tuple1& tuple1, const Tuple2& tuple2, const Tuple3& comparisons) {
    return compareTuplesImpl(tuple1, tuple2, comparisons, std::make_index_sequence<TupleSize>{});
  }
};

} // namespace eicrecon
