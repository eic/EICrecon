// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024, Nathan Brei, Dmitry Kalinkin

#pragma once

#include <algorithms/logger.h>
#include "PIDLookupTable.h"
#include <JANA/Services/JServiceLocator.h>
#include <JANA/JLogger.h>
#include <mutex>
#include <filesystem>

namespace eicrecon {

class PIDLookupTableSvc : public algorithms::LoggedService<PIDLookupTableSvc> {
public:
  void init() {};

  const PIDLookupTable* load(std::string filename, const PIDLookupTable::Binning& binning) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto pair = m_cache.find(filename);
    if (pair == m_cache.end()) {
      auto lut = std::make_unique<PIDLookupTable>();
      info("Loading PID lookup table \"{}\"", filename);

      if (!std::filesystem::exists(filename)) {
        error("PID lookup table \"{}\" not found", filename);
        return nullptr;
      }

      lut->load_file(filename, binning); // load_file can except
      auto result_ptr = lut.get();
      m_cache.insert({filename, std::move(lut)});
      return result_ptr;
    } else {
      return pair->second.get();
    }
  }

private:
  std::mutex m_mutex;
  std::map<std::string, std::unique_ptr<PIDLookupTable>> m_cache;

  ALGORITHMS_DEFINE_LOGGED_SERVICE(PIDLookupTableSvc);
};

} // namespace eicrecon
