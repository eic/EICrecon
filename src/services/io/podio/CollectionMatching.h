// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, ePIC Collaboration

#pragma once

#include <cstddef>
#include <regex>
#include <set>
#include <string>
#include <vector>

namespace eicrecon {

struct CollectionMatch {
  std::vector<std::string> matched;            // matching and not excluded, in sorted order
  std::vector<std::string> unmatched_patterns; // patterns that match no collection
};

// Match collection names against regex patterns in one pass
inline CollectionMatch match_collections(const std::set<std::string>& collections,
                                         const std::set<std::string>& patterns,
                                         const std::set<std::string>& excluded) {
  std::vector<std::regex> regexes(patterns.begin(), patterns.end());
  std::vector<bool> used(regexes.size(), false);

  CollectionMatch result;
  for (const auto& c : collections) {
    bool matched = false;
    for (std::size_t i = 0; i < regexes.size(); ++i) {
      // once c matched, only patterns that matched nothing yet need testing
      if (matched && used[i]) {
        continue;
      }
      if (std::regex_match(c, regexes[i])) {
        matched = true;
        used[i] = true;
      }
    }
    if (matched && !excluded.contains(c)) {
      result.matched.push_back(c);
    }
  }

  std::size_t i = 0;
  for (const auto& p : patterns) {
    if (!used[i++]) {
      result.unmatched_patterns.push_back(p);
    }
  }
  return result;
}

} // namespace eicrecon
