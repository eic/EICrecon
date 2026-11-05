// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 EICrecon authors

#pragma once

#include <string>
#include "PandoraPFAConfig.h"

namespace eicrecon {

/**
 * @brief Apply parameter overrides to Pandora XML settings
 *
 * This utility modifies XML content by substituting parameter values based on
 * the PandoraPFAConfig structure. Parameters with values > 0 (or >= 0 for int)
 * override the corresponding XML values. Parameters with 0 or negative values
 * preserve the XML defaults.
 *
 * @param xmlContent The original XML settings content
 * @param cfg Configuration containing parameter overrides
 * @return Modified XML content with overrides applied
 */
std::string applyXmlParameterOverrides(const std::string& xmlContent, const PandoraPFAConfig& cfg);

} // namespace eicrecon
