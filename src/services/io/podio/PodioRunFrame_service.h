// Copyright 2025, Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//
// Service to share the PODIO run metadata frame with factories.

#pragma once

#include <JANA/JApplicationFwd.h>
#include <JANA/JServiceFwd.h>
#include <podio/Frame.h>
#include <memory>
#include <string>
#include <iostream>
#include <optional>
#include <vector>
#include <spdlog/logger.h>

/// Service that caches and shares the PODIO run metadata frame
/// from the input file with downstream factories.
///
/// Factories can retrieve the run frame to access run-level metadata
/// and configure themselves accordingly.
///
/// This service reads the metadata independently during initialization,
/// defaulting to the first event file added to the application but with
/// an option to source metadata from a different file via the
/// 'podio:metadata_file' parameter.
class PodioRunFrame_service : public JService {
public:
  explicit PodioRunFrame_service(JApplication* app);
  ~PodioRunFrame_service() override = default;

  /// Set the run metadata frame to be shared with factories
  /// (internal use - the frame is read automatically during initialization)
  void SetRunFrame(std::shared_ptr<podio::Frame> frame);

  /// Get the cached run metadata frame
  std::shared_ptr<const podio::Frame> GetRunFrame() const;

  /// Get a specific collection from the run frame by type and name
  template <typename CollT>
  const CollT* GetCollection(const std::string& name) const {
    auto frame = m_frame;
    if (!frame) {
      return nullptr;
    }
    return &frame->get<CollT>(name);
  }

  /// Get a parameter value from the run frame. Returns std::nullopt if frame or parameter not found.
  template <typename T>
  std::optional<T> GetParameter(const std::string& key) const {
    if (!m_frame) {
      return std::nullopt;
    }
    return m_frame->getParameter<T>(key);
  }

  /// Convenience: retrieve a numeric parameter as double, regardless of storage type.
  /// Tries double, float, string (parsed), and first element of vector forms in that order.
  std::optional<double> GetParameterAsDouble(const std::string& key) const;

  /// Print all parameters and their values from the run frame
  void PrintParameters() const;

private:
  JApplication* m_app = nullptr;
  mutable std::shared_ptr<const podio::Frame> m_frame;
  std::shared_ptr<spdlog::logger> m_log;

  /// Attempt to read the run metadata frame from a PODIO file
  void ReadRunFrame() const;

  /// Get the input PODIO filename from event sources or parameter
  std::string GetInputFilename() const;
};
