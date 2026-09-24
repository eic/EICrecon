// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 EICrecon authors

#include "XmlParameterOverride.h"

#include <regex>
#include <sstream>
#include <iomanip>

namespace eicrecon {

namespace {

  /**
 * @brief Replace XML element value if condition is met
 *
 * @param xmlContent XML string to modify
 * @param elementName XML element name (tag)
 * @param newValue New value to substitute
 * @param shouldApply Whether to apply the override
 * @return Modified XML content
 */
  std::string replaceXmlElement(const std::string& xmlContent, const std::string& elementName,
                                const std::string& newValue, bool shouldApply) {
    if (!shouldApply) {
      return xmlContent;
    }

    // Pattern: <ElementName>any_value</ElementName>
    // Captures the opening tag, replaces the content, preserves closing tag
    std::string pattern = "(<" + elementName + ">)[^<]*(</" + elementName + ">)";
    std::regex regex(pattern);

    std::string replacement = "$1" + newValue + "$2";
    return std::regex_replace(xmlContent, regex, replacement);
  }

} // anonymous namespace

std::string applyXmlParameterOverrides(const std::string& xmlContent, const PandoraPFAConfig& cfg) {
  std::string modifiedXml = xmlContent;
  std::ostringstream valueStream;

  // Topological clustering parameters
  if (cfg.topologicalMaxCaloHitSeparation > 0) {
    valueStream.str("");
    valueStream << cfg.topologicalMaxCaloHitSeparation;
    modifiedXml = replaceXmlElement(modifiedXml, "TopologicalAssociationMaxCaloHitSeparation",
                                    valueStream.str(), true);
  }

  if (cfg.topologicalMaxClusterSeparation > 0) {
    valueStream.str("");
    valueStream << cfg.topologicalMaxClusterSeparation;
    modifiedXml = replaceXmlElement(modifiedXml, "TopologicalAssociationMaxClusterSeparation",
                                    valueStream.str(), true);
  }

  if (cfg.topologicalMaxClusterCosAngle > 0) {
    valueStream.str("");
    valueStream << std::fixed << std::setprecision(4) << cfg.topologicalMaxClusterCosAngle;
    modifiedXml = replaceXmlElement(modifiedXml, "TopologicalAssociationMaxClusterCosAngle",
                                    valueStream.str(), true);
  }

  // Track-cluster association parameters
  if (cfg.trackClusterMaxCaloHitSeparation > 0) {
    valueStream.str("");
    valueStream << cfg.trackClusterMaxCaloHitSeparation;
    modifiedXml = replaceXmlElement(modifiedXml, "TrackClusterAssociationMaxCaloHitSeparation",
                                    valueStream.str(), true);
  }

  if (cfg.trackClusterMaxSeparationFromTrack > 0) {
    valueStream.str("");
    valueStream << cfg.trackClusterMaxSeparationFromTrack;
    modifiedXml = replaceXmlElement(modifiedXml, "TrackClusterAssociationMaxSeparationFromTrack",
                                    valueStream.str(), true);
  }

  // Neutral PFO creation parameters
  if (cfg.neutralPfoMinClusterEnergy > 0) {
    valueStream.str("");
    valueStream << std::fixed << std::setprecision(3) << cfg.neutralPfoMinClusterEnergy;
    modifiedXml = replaceXmlElement(modifiedXml, "MinClusterEnergy", valueStream.str(), true);
  }

  return modifiedXml;
}

} // namespace eicrecon
