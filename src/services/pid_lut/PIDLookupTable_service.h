// Copyright 2024, Nathan Brei
// Subject to the terms in the LICENSE file found in the top-level directory.

#pragma once


#include "PIDLookupTable.h"
#include <JANA/Services/JServiceLocator.h>
#include <mutex>


class PIDLookupTable_service : public JService {

    std::mutex m_mutex;
    std::map<std::string, PIDLookupTable*> m_cache;

public:

    const PIDLookupTable& FetchTable(std::string filename) {

        std::lock_guard<std::mutex> lock(m_mutex);
        auto pair = m_cache.find(filename);
        if (pair == m_cache.end()) {
            auto lut = new PIDLookupTable;
            lut->LoadFile(filename);
            m_cache.insert({filename, lut});
            return *lut;
        }
        else {
            return *(pair.second);
        }
    }
};
