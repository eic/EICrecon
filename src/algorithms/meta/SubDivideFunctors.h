// Copyright 2024, Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//
#pragma once

#include <JANA/JApplication.h>
#include <DDSegmentation/BitFieldCoder.h>
#include <algorithms/geo.h>

namespace eicrecon {

// ----------------------------------------------------------------------------
// Functor to split collection based on a range of values
// ----------------------------------------------------------------------------
template <auto MemberFunctionPtr>
class RangeSplit {
public:

    RangeSplit(std::vector<std::pair<long int,long int>> ranges) : m_ranges(ranges) {};

    template <typename T>
    std::vector<int> operator()(T& instance) const {
        std::vector<int> ids;
        //Check if requested value is within the ranges
        for(size_t i = 0; i < m_ranges.size(); i++){
            if((instance.*MemberFunctionPtr)() > m_ranges[i].first && (instance.*MemberFunctionPtr)() < m_ranges[i].second){
                ids.push_back(i);
            }
        }
        return ids;
    }

private:
    std::vector<std::pair<long int,long int>> m_ranges;

};

// ----------------------------------------------------------------------------
// Functor to split collection based on geometry
// ----------------------------------------------------------------------------
class GeometrySplit {
public:

    GeometrySplit(std::vector<std::vector<long int>> ids, std::string readout, std::vector<std::string> divisions)
    : m_ids(ids), m_readout(readout), m_divisions(divisions){};

    template <typename T>
    std::vector<int> operator()(T& instance) const {
        if(!is_init){
            m_id_dec = algorithms::GeoSvc::instance().detector()->readout(m_readout).idSpec().decoder();
            for (auto d : m_divisions){
                m_div_ids.push_back(m_id_dec->index(d));
            }
            is_init = true;
        }

        std::vector<int> ids;
        //Check if requested value is within the ranges
        auto cellID = instance.getCellID();
        std::vector<long int> det_ids;
        for(auto d : m_div_ids){
            det_ids.push_back(m_id_dec->get(cellID, d));
        }
        auto index = std::find(m_ids.begin(),m_ids.end(),det_ids);
        if(index != m_ids.end()){
            ids.push_back(index-m_ids.begin());
        }
        return ids;
    }

private:
    
    //const dd4hep::Detector* m_detector{};
    mutable bool is_init{false};
    mutable dd4hep::DDSegmentation::BitFieldCoder* m_id_dec;
    std::vector<std::vector<long int>> m_ids;
    mutable std::vector<size_t> m_div_ids;
    std::vector<std::string> m_divisions;
    std::string m_readout;

};


// ----------------------------------------------------------------------------
// Functor to split collection based on any number of collection values
// ----------------------------------------------------------------------------
template <auto... MemberFunctionPtrs>
class ValueSplit {
public:

    ValueSplit(std::vector<std::vector<int>> ids) : m_ids(ids) {};

    template <typename T>
    std::vector<int> operator()(T& instance) const {
        std::vector<int> ids;
        //Check if requested value is within the ranges
        std::vector<int> values;
        (values.push_back((instance.*MemberFunctionPtrs)()), ...);
        auto index = std::find(m_ids.begin(),m_ids.end(),values);
        if(index != m_ids.end()){
            ids.push_back(index-m_ids.begin());
        }
        return ids;
    }

private:
    std::vector<std::vector<int>> m_ids;

};

} // eicrecon
