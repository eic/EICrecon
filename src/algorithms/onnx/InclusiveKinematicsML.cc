// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2022, 2023 Wouter Deconinck, Tooba Ali

#include <onnxruntime_cxx_api.h>

#include "InclusiveKinematicsML.h"

namespace eicrecon {

  void InclusiveKinematicsML::init(std::shared_ptr<spdlog::logger>& logger) {
  }

  void InclusiveKinematicsML::process(
      const InclusiveKinematicsML::Input& input,
      const InclusiveKinematicsML::Output& output) const {

    const auto [electron, da] = input;
    auto [ml] = output;

    const auto& api = Ort::GetApi();

    Ort::Session session(nullptr);
  }

} // namespace eicrecon
