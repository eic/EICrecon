// Copyright 2024, Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//
#pragma once

#include <algorithms/geo.h>

namespace eicrecon {

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
  RangeSplit(std::vector<std::pair<double, double>> ranges) : m_ranges(ranges) {};

  template <typename T> std::vector<std::size_t> operator()(T& instance) const {
    std::vector<std::size_t> ids;
    auto value = ChainInvoker<MemberFunctionPtrs...>::invoke(instance);
    // Check if requested value is within the ranges
    for (std::size_t i = 0; i < m_ranges.size(); i++) {
      if (value > m_ranges[i].first && value < m_ranges[i].second) {
        ids.push_back(i);
      }
    }
    return ids;
  }

private:
  std::vector<std::pair<double, double>> m_ranges;
};

// ----------------------------------------------------------------------------
// Functor to split collection based on geometry
// ----------------------------------------------------------------------------
class GeometrySplit {
public:
  GeometrySplit(std::vector<std::vector<long int>> ids, std::string readout,
                std::vector<std::string> divisions)
      : m_ids(ids)
      , m_divisions(divisions)
      , m_readout(readout)
      , is_init(std::make_shared<std::once_flag>())
      , m_id_dec(std::make_shared<dd4hep::DDSegmentation::BitFieldCoder*>())
      , m_div_ids(std::make_shared<std::vector<std::size_t>>()) {};

  template <typename T> std::vector<std::size_t> operator()(T& instance) const {

    // Initialize the decoder and division ids on the first function call
    std::call_once(*is_init, &GeometrySplit::init, this);

    //Check which detector division to put the hit into
    auto cellID = instance.getCellID();
    std::vector<long int> det_ids;
    for (auto d : *m_div_ids) {
      det_ids.push_back((*m_id_dec)->get(cellID, d));
    }

    auto index = std::find(m_ids.begin(), m_ids.end(), det_ids);

    std::vector<std::size_t> ids;
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

  std::vector<std::vector<long int>> m_ids;
  std::vector<std::string> m_divisions;
  std::string m_readout;

  std::shared_ptr<std::once_flag> is_init;
  std::shared_ptr<dd4hep::DDSegmentation::BitFieldCoder*> m_id_dec;
  std::shared_ptr<std::vector<std::size_t>> m_div_ids;
};

// ----------------------------------------------------------------------------
// Functor to split collection based on any number of collection values
// ----------------------------------------------------------------------------
template <auto... MemberFunctionPtrs> class ValueSplit {
public:
  ValueSplit(std::vector<std::vector<int>> ids) : m_ids(ids) {};

  template <typename T> std::vector<std::size_t> operator()(T& instance) const {
    std::vector<std::size_t> ids;
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
  std::vector<std::vector<int>> m_ids;
};

} // namespace eicrecon
