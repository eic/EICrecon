// Copyright 2023, Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//
#pragma once

#include <JANA/JApplication.h>
#include <DDSegmentation/BitFieldCoder.h>

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

    GeometrySplit(JApplication* app,std::vector<std::vector<long int>> ids, std::string readout, std::vector<std::string> div) : m_ids(ids) {
        m_id_dec = app->GetService<DD4hep_service>()->detector()->readout(readout).idSpec().decoder();
        for (auto d : div){
            m_div.push_back(m_id_dec->index(d));
        }
    };

    template <typename T>
    std::vector<int> operator()(T& instance) const {
        std::vector<int> ids;
        //Check if requested value is within the ranges
        auto cellID = instance.getCellID();
        std::vector<long int> det_ids {m_id_dec->get(cellID, m_div[0]),m_id_dec->get(cellID, m_div[1])};
        auto index = std::find(m_ids.begin(),m_ids.end(),det_ids);
        if(index != m_ids.end()){
            ids.push_back(index-m_ids.begin());
        }
        return ids;
    }

private:
    dd4hep::DDSegmentation::BitFieldCoder* m_id_dec;
    std::vector<std::vector<long int>> m_ids;
    std::vector<size_t> m_div;

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
