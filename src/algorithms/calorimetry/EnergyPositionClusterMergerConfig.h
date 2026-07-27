// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2023 Wouter Deconinck

#pragma once
#include <DD4hep/DD4hepUnits.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace eicrecon {

/// Which cluster to use as the position source (or comparison target) in a PositionRule.
enum class PositionSource { pc1, pc2, ec };

inline std::ostream& operator<<(std::ostream& out, const PositionSource& s) {
  switch (s) {
    case PositionSource::pc1: out << "pc1"; break;
    case PositionSource::pc2: out << "pc2"; break;
    case PositionSource::ec:  out << "ec";  break;
    default: out.setstate(std::ios::failbit);
  }
  return out;
}

inline std::istream& operator>>(std::istream& in, PositionSource& s) {
  std::string token;
  in >> token;
  if      (token == "pc1") s = PositionSource::pc1;
  else if (token == "pc2") s = PositionSource::pc2;
  else if (token == "ec")  s = PositionSource::ec;
  else in.setstate(std::ios::failbit);
  return in;
}

/// A single position-source rule evaluated in priority order.
/// All enabled conditions must be satisfied for the rule to fire.
/// A field value < 0 means that condition is disabled.
///
/// Serialized format: source:compareSource:minEnergy:maxEnergy:maxDphi
/// Example: "ec:pc2:0.5:-1:0.3"
struct PositionRule {
  /// Cluster to use as the output position if this rule fires.
  PositionSource source = PositionSource::pc1;

  /// Cluster to compare against ec when evaluating maxDphi.
  /// Ignored when maxDphi < 0.
  PositionSource compareSource = PositionSource::pc1;

  /// Rule fires only when ec.energy >= minEnergy. Disabled if < 0.
  double minEnergy = -1;

  /// Rule fires only when ec.energy < maxEnergy. Disabled if < 0.
  double maxEnergy = -1;

  /// Rule fires only when |dphi(compareSource, ec)| > maxDphi. Disabled if < 0.
  double maxDphi = -1;

  friend bool operator==(const PositionRule& a, const PositionRule& b) {
    return a.source == b.source && a.compareSource == b.compareSource &&
           a.minEnergy == b.minEnergy && a.maxEnergy == b.maxEnergy &&
           a.maxDphi == b.maxDphi;
  }
};

inline std::ostream& operator<<(std::ostream& out, const PositionRule& r) {
  out << r.source << ":" << r.compareSource << ":"
      << r.minEnergy << ":" << r.maxEnergy << ":" << r.maxDphi;
  return out;
}

inline std::istream& operator>>(std::istream& in, PositionRule& r) {
  // Rules are serialized as colon-separated fields.
  // JANA splits the CLI vector on commas, so each token reaching here
  // is one full rule string, e.g. "ec:pc2:0.5:-1:0.3".
  // We replace ':' with spaces and re-parse via a stringstream.
  std::string token;
  in >> token;
  for (char& c : token) {
    if (c == ':') c = ' ';
  }
  std::istringstream ss(token);
  PositionRule tmp;
  if (!(ss >> tmp.source >> tmp.compareSource >>
        tmp.minEnergy >> tmp.maxEnergy >> tmp.maxDphi)) {
    in.setstate(std::ios::failbit);
    return in;
  }
  r = tmp;
  return in;
}

struct EnergyPositionClusterMergerConfig {

  double energyRelTolerance{0.5};
  double phiTolerance{0.1};
  double etaTolerance{0.2};

  /// Ordered list of position-source rules.
  /// Rules are evaluated in order; the first rule whose conditions all pass wins.
  /// If no rule fires, pc1 is used as the fallback.
  /// CLI format: comma-separated rules, each rule is source:compareSource:minEnergy:maxEnergy:maxDphi
  /// Example: -PPrefix:positionRules=ec:pc2:0.5:-1:0.3,pc2:pc2:0.5:-1:-1
  std::vector<PositionRule> positionRules{};
};

} // namespace eicrecon
