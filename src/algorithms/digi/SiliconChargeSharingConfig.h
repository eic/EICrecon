// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2024 Souvik Paul

#pragma once

namespace eicrecon {

struct SiliconChargeSharingConfig {
  // Parameters of Silicon signal generation
  // determines the meaning of sigma_sharingx and y.
  // rel means relative, so charge sharing range = sigma_sharingx * cell_width_x, etc for y
  // abs means absolute, charge sharing range = sigma_shargeingx directly
  enum ESigmaMode { abs = 0, rel = 1 } sigma_mode = abs;
  float sigma_sharingx;
  float sigma_sharingy;
  float min_edep;
  std::string readout;
};

std::istream& operator>>(std::istream& in, SiliconChargeSharingConfig::ESigmaMode& sigmaMode) {
  std::string s;
  in >> s;
  // stringifying the enums causes them to be converted to integers before conversion to strings
  if (s == "abs" or s == "0") {
    sigmaMode = SiliconChargeSharingConfig::ESigmaMode::abs;
  } else if (s == "rel" or s == "1") {
    sigmaMode = SiliconChargeSharingConfig::ESigmaMode::rel;
  } else {
    in.setstate(std::ios::failbit); // Set the fail bit if the input is not valid
  }

  return in;
}
std::ostream& operator<<(std::ostream& out, SiliconChargeSharingConfig::ESigmaMode& sigmaMode) {
  switch (sigmaMode) {
  case SiliconChargeSharingConfig::ESigmaMode::abs:
    out << "abs";
    break;
  case SiliconChargeSharingConfig::ESigmaMode::rel:
    out << "rel";
    break;
  default:
    out.setstate(std::ios::failbit);
  }
  return out;
}

} // namespace eicrecon
