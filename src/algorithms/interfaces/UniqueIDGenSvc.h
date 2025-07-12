// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2014-2024 Key4hep-Project
// Adapted from:
// - https://github.com/key4hep/k4FWCore/blob/cc6a28a245592a5b54667925a4b2e3809e6eba54/k4FWCore/components/UniqueIDGenSvc.h
// - https://github.com/key4hep/k4FWCore/blob/cc6a28a245592a5b54667925a4b2e3809e6eba54/k4FWCore/components/UniqueIDGenSvc.cpp
// Local modifications:
// Copyright (C) 2025 Wouter Deconinck

#pragma once

#include <edm4hep/EventHeaderCollection.h>
#include <algorithms/logger.h>
#include <algorithms/service.h>
#include <bitset>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

namespace algorithms {

class UniqueIDGenSvc : public LoggedService<UniqueIDGenSvc> {
public:
  using event_num_t = decltype(std::declval<edm4hep::EventHeader>().getEventNumber());
  using run_num_t   = decltype(std::declval<edm4hep::EventHeader>().getRunNumber());

  using seed_t = uint64_t;

private:
  static constexpr size_t event_num_digits = std::numeric_limits<event_num_t>::digits;
  static constexpr size_t run_num_digits   = std::numeric_limits<run_num_t>::digits;
  static constexpr size_t seed_digits      = std::numeric_limits<seed_t>::digits;
  static constexpr size_t name_digits      = std::numeric_limits<size_t>::digits;

public:
  virtual void init() {}

  // calculate a unique id from name and the event header
  size_t getUniqueID(const edm4hep::EventHeader& evt_header, const std::string_view& name) const {
    return getUniqueID(evt_header.getEventNumber(), evt_header.getRunNumber(), name);
  }
  // calculate a unique id from name and the first element of collection
  size_t getUniqueID(const edm4hep::EventHeaderCollection& evt_headers,
                     const std::string_view& name) const {
    return getUniqueID(evt_headers.at(0), name);
  }
  // calculate a unique id from name and the event and run numbers
  size_t getUniqueID(const event_num_t evt_num, const run_num_t run_num,
                     const std::string_view& name) const {
    std::bitset<seed_digits> seed_bits           = m_seed.value();
    std::bitset<event_num_digits> event_num_bits = evt_num;
    std::bitset<run_num_digits> run_num_bits     = run_num;
    std::bitset<name_digits> name_bits           = std::hash<std::string_view>{}(name);

    std::bitset<seed_digits + event_num_digits + run_num_digits + name_digits> combined_bits;

    for (size_t i = 0; i < name_digits; i++) {
      combined_bits[i] = name_bits[i];
    }
    for (size_t i = 0; i < run_num_digits; i++) {
      combined_bits[i + name_digits] = run_num_bits[i];
    }
    for (size_t i = 0; i < event_num_digits; i++) {
      combined_bits[i + run_num_digits + name_digits] = event_num_bits[i];
    }
    for (size_t i = 0; i < seed_digits; i++) {
      combined_bits[i + event_num_digits + run_num_digits + name_digits] = seed_bits[i];
    }

    auto hash =
        std::hash<std::bitset<seed_digits + event_num_digits + run_num_digits + name_digits>>{}(
            combined_bits);

    if (m_checkDuplicates) {
      auto [it, inserted] = [=, this, &name]() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_uniqueIDs.insert({hash, {evt_num, run_num, std::string{name}}});
      }();
      if (!inserted) {
        const auto& [id_evt, id_run, id_name] = it->second;
        // TODO log to error
        throw std::runtime_error(fmt::format(
            "Duplicate ID for event number, run number and algorithm name: {}, {}, \"{}\". "
            "ID already assigned to: {}, {}, \"{}\"",
            evt_num, run_num, name, id_evt, id_run, id_name));
      }
    }

    return hash;
  };

protected:
  ALGORITHMS_DEFINE_LOGGED_SERVICE(UniqueIDGenSvc)

private:
  Property<size_t> m_seed{this, "seed", 1, "Random seed for the internal random engine"};
  Property<bool> m_checkDuplicates{
      this, "checkDuplicates", false,
      "Caches obtained ID and throws an exception if a duplicate would be returned"};

  mutable std::unordered_map<size_t, std::tuple<event_num_t, run_num_t, std::string>, std::identity>
      m_uniqueIDs;
  mutable std::mutex m_mutex;
};

} // namespace algorithms
