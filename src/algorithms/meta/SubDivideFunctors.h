// Copyright 2024, Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//
#pragma once

#include <algorithms/geo.h>

namespace eicrecon {

// ----------------------------------------------------------------------------
// Functor to split collection based on a range of values
// ----------------------------------------------------------------------------
template <auto MemberFunctionPtr>
class RangeSplit {
public:
    RangeSplit(const std::vector<std::pair<double, double>>& ranges, bool inside = true)
        : m_ranges(ranges), m_inside(ranges.size(), inside) {}

    RangeSplit(const std::vector<std::pair<double, double>>& ranges, const std::vector<bool>& inside)
        : m_ranges(ranges) {
        if (inside.size() != ranges.size()) {
            throw std::invalid_argument("Size of inside must match the size of ranges");
        }
        m_inside = inside;
    }

    template <typename T>
    std::vector<int> operator()(T& instance) const {
        std::vector<int> ids;
        //Check if requested value is within the ranges
        for(size_t i = 0; i < m_ranges.size(); i++){
            if(m_inside[i]){
                if((instance.*MemberFunctionPtr)() >= m_ranges[i].first && (instance.*MemberFunctionPtr)() <= m_ranges[i].second){
                    ids.push_back(i);
                }
            } else {
                if((instance.*MemberFunctionPtr)() < m_ranges[i].first || (instance.*MemberFunctionPtr)() > m_ranges[i].second){
                    ids.push_back(i);
                }
            }
        }
        return ids;
    }

private:
    const std::vector<std::pair<double,double>>& m_ranges;
    std::vector<bool> m_inside;

};

// ----------------------------------------------------------------------------
// Functor to split collection based on geometry
// ----------------------------------------------------------------------------
class GeometrySplit {
public:

    GeometrySplit(const std::vector<std::vector<long int>>& ids, const std::string& readout, const std::vector<std::string>& divisions)
    : m_ids(ids), m_readout(readout), m_divisions(divisions){};

    template <typename T>
    std::vector<int> operator()(T& instance) const {

        // Initialize the decoder and division ids on the first function call
        std::call_once(*is_init, &GeometrySplit::init, this);

        //Check which detector division to put the hit into
        auto cellID = instance.getCellID();
        std::vector<long int> det_ids;
        for(auto d : m_div_ids){
            det_ids.push_back(m_id_dec->get(cellID, d));
        }
        auto index = std::find(m_ids.begin(),m_ids.end(),det_ids);

        std::vector<int> ids;
        if(index != m_ids.end()){
            ids.push_back(std::distance(m_ids.begin(),index));
        }
        return ids;
    }

private:

    void init() const {
        m_id_dec = algorithms::GeoSvc::instance().detector()->readout(m_readout).idSpec().decoder();
        for (auto d : m_divisions){
            m_div_ids.push_back(m_id_dec->index(d));
        }
    }

    const std::vector<std::vector<long int>>& m_ids;
    const std::vector<std::string>& m_divisions;
    const std::string& m_readout;

    mutable std::shared_ptr<std::once_flag> is_init = std::make_shared<std::once_flag>();
    mutable dd4hep::DDSegmentation::BitFieldCoder* m_id_dec;
    mutable std::vector<size_t> m_div_ids;

};


// ----------------------------------------------------------------------------
// Functor to split collection based on any number of collection values
// ----------------------------------------------------------------------------
template <auto... MemberFunctionPtrs>
class ValueSplit {
public:

    ValueSplit(const std::vector<std::vector<int>>& ids, bool matching = true)
        : m_ids(ids), m_matching(matching) {};

    template <typename T>
    std::vector<int> operator()(T& instance) const {
        std::vector<int> ids;
        // Check if requested value matches any configuration combinations
        std::vector<int> values;
        (values.push_back((instance.*MemberFunctionPtrs)()), ...);
        if(m_matching){
            auto index = std::find(m_ids.begin(),m_ids.end(),values);
            if(index != m_ids.end()){
                ids.push_back(std::distance(m_ids.begin(),index));
            }
        } else {
            for (size_t i = 0; i < m_ids.size(); ++i){
                if(m_ids[i] != values){
                    ids.push_back(i);
                }
            }
        }
        return ids;
    }

private:
    const std::vector<std::vector<int>>& m_ids;
    bool m_matching = true;

};

} // eicrecon
