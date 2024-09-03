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

    RangeSplit(std::vector<std::pair<double,double>> ranges) : m_ranges(ranges) {};

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
    std::vector<std::pair<double,double>> m_ranges;

};

// ----------------------------------------------------------------------------
// Functor to split collection based on geometry
// ----------------------------------------------------------------------------
class GeometrySplit {
public:

    GeometrySplit(std::vector<std::vector<long int>> ids, std::string readout, std::vector<std::string> divisions)
    : m_ids(ids), m_readout(readout), m_divisions(divisions),
      is_init(std::make_shared<std::once_flag>()),
      m_id_dec(std::make_shared<dd4hep::DDSegmentation::BitFieldCoder*>()),
      m_div_ids(std::make_shared<std::vector<size_t>>()) {};

    // // Copy constructor without std::call_once and initialization
    // GeometrySplit(const GeometrySplit& other)
    // : m_ids(other.m_ids), m_readout(other.m_readout), m_divisions(other.m_divisions), m_div_ids(other.m_div_ids), is_init() {  }

    // // Copy assignment operator without std::call_once and initialization
    // GeometrySplit& operator=(const GeometrySplit& other) {
    //     if (this != &other) {
    //         m_ids = other.m_ids;
    //         m_readout = other.m_readout;
    //         m_divisions = other.m_divisions;
    //         m_div_ids = other.m_div_ids;
    //     }
    //     return *this;
    // }

    template <typename T>
    std::vector<int> operator()(T& instance) const {

        // Initialize the decoder and division ids on the first function call
        std::call_once(*is_init, &GeometrySplit::init, this);

        //Check which detector division to put the hit into
        auto cellID = instance.getCellID();
        std::cout << "Cell ID: " << cellID << std::endl;
        std::vector<long int> det_ids;
        for(auto d : *m_div_ids){
            std::cout << "Division: " << d << std::endl;
            det_ids.push_back((*m_id_dec)->get(cellID, d));
        }
        //print out the detector ids
        std::cout << "Module Layer" << std::endl;
        for(auto id : det_ids){
            std::cout << id << " ";
        }
        std::cout << m_id_dec << std::endl;
        std::cout << std::endl;
        

        auto index = std::find(m_ids.begin(),m_ids.end(),det_ids);

        std::vector<int> ids;
        if(index != m_ids.end()){
            ids.push_back(std::distance(m_ids.begin(),index));
        }
        return ids;
    }

private:

    void init() const {
        std::cout << "Initializing GeometrySplit" << std::endl;
        *m_id_dec = algorithms::GeoSvc::instance().detector()->readout(m_readout).idSpec().decoder();
        m_div_ids->clear();
        for (auto d : m_divisions){
            m_div_ids->push_back((*m_id_dec)->index(d));
        }
    }

    std::vector<std::vector<long int>> m_ids;
    std::vector<std::string> m_divisions;
    std::string m_readout;

    std::shared_ptr<std::once_flag> is_init;
    std::shared_ptr<dd4hep::DDSegmentation::BitFieldCoder*> m_id_dec = nullptr;
    std::shared_ptr<std::vector<size_t>> m_div_ids;

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
        // Check if requested value matches any configuration combinations
        std::vector<int> values;
        (values.push_back((instance.*MemberFunctionPtrs)()), ...);
        auto index = std::find(m_ids.begin(),m_ids.end(),values);
        if(index != m_ids.end()){
            ids.push_back(std::distance(m_ids.begin(),index));
        }
        return ids;
    }

private:
    std::vector<std::vector<int>> m_ids;

};

} // eicrecon
