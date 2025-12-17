// Copyright 2025, Simon Gardner
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#include "PodioRunFrame_service.h"
#include <JANA/JApplication.h>
#include <JANA/Services/JComponentManager.h>
#include <podio/ROOTReader.h>
#include "services/log/Log_service.h"
#include <iostream>
#include <iomanip>
#include <tuple>
#include <cstdlib>

PodioRunFrame_service::PodioRunFrame_service(JApplication* app) : m_app(app) {
  // Get logger
  m_log = m_app->GetService<Log_service>()->logger("PodioRunFrame_service");

  m_log->debug("PodioRunFrame_service initializing");
  // Do not force-read here: event sources may not be registered yet.
  // We'll lazy-load on first GetRunFrame()/PrintParameters call.
}

void PodioRunFrame_service::SetRunFrame(std::shared_ptr<podio::Frame> frame) {
  m_frame = std::move(frame);
}

std::shared_ptr<const podio::Frame> PodioRunFrame_service::GetRunFrame() const {
  if (!m_frame) {
    // Attempt a lazy load in case event sources or parameters were not ready earlier
    ReadRunFrame();
  }
  return m_frame;
}

std::string PodioRunFrame_service::GetInputFilename() const {
  // First, check if user specified a metadata file explicitly
  auto param_mgr = m_app->GetJParameterManager();
  if (param_mgr) {
    auto metadata_file_param = param_mgr->FindParameter("podio:metadata_file");
    if (metadata_file_param) {
      std::string metadata_file = m_app->GetParameterValue<std::string>("podio:metadata_file");
      if (!metadata_file.empty()) {
        m_log->debug("Using metadata file from podio:metadata_file parameter: {}", metadata_file);
        return metadata_file;
      }
    }
  }

  // Fall back to the first event source
  try {
    auto component_mgr = m_app->GetService<JComponentManager>();
    auto event_sources = component_mgr->get_evt_srces();
    if (!event_sources.empty()) {
      std::string filename = event_sources[0]->GetResourceName();
      m_log->debug("Using first event source as metadata file: {}", filename);
      return filename;
    }
  } catch (const std::exception& e) {
    m_log->debug("Could not get event sources: {}", e.what());
  }

  m_log->warn("No event source or metadata file found");
  return "";
}

void PodioRunFrame_service::ReadRunFrame() const {
  std::string filename = GetInputFilename();
  if (filename.empty()) {
    // Keep this at debug to avoid spam before event sources exist
    m_log->debug("No filename available for reading run metadata (yet)");
    return;
  }

  try {
    podio::ROOTReader reader;
    reader.openFile(filename);

    // Check if runs branch exists
    auto runs_entries = reader.getEntries("runs");
    if (runs_entries == 0) {
      m_log->warn("No 'runs' frame found in PODIO file: {}", filename);
      return;
    }

    // Read the first entry from the "runs" branch
    auto run_frame_data = reader.readEntry("runs", 0);
    m_frame             = std::make_shared<podio::Frame>(std::move(run_frame_data));

    m_log->debug("Successfully read PODIO run metadata from: {}", filename);
  } catch (const std::exception& e) {
    m_log->warn("Failed to read PODIO run metadata from {}: {}", filename, e.what());
  }
}

std::optional<double> PodioRunFrame_service::GetParameterAsDouble(const std::string& key) const {
  auto frame = GetRunFrame();
  if (!frame)
    return std::nullopt;

  if (auto d = frame->getParameter<double>(key))
    return d;
  if (auto f = frame->getParameter<float>(key))
    return static_cast<double>(*f);
  if (auto s = frame->getParameter<std::string>(key)) {
    try {
      return std::stod(*s);
    } catch (...) {
      return std::nullopt;
    }
  }
  if (auto dv = frame->getParameter<std::vector<double>>(key)) {
    if (!dv->empty())
      return (*dv)[0];
  }
  if (auto fv = frame->getParameter<std::vector<float>>(key)) {
    if (!fv->empty())
      return static_cast<double>((*fv)[0]);
  }
  if (auto sv = frame->getParameter<std::vector<std::string>>(key)) {
    if (!sv->empty()) {
      try {
        return std::stod((*sv)[0]);
      } catch (...) {
        return std::nullopt;
      }
    }
  }
  return std::nullopt;
}

void PodioRunFrame_service::PrintParameters() const {
  // Ensure the frame is loaded
  auto frame = GetRunFrame();
  if (!frame) {
    std::cout << "No run frame available\n";
    return;
  }

  std::cout << "\n=== PODIO Run Frame Parameters ===\n\n";

  const auto& params = frame->getParameters();

  // Integers
  {
    auto [keys, values] = params.getKeysAndValues<int>();
    if (!keys.empty()) {
      std::cout << "Integer Parameters:\n";
      for (size_t i = 0; i < keys.size(); ++i) {
        std::cout << "  " << std::setw(30) << std::left << keys[i] << " = ";
        if (values[i].size() == 1) {
          std::cout << values[i][0] << "\n";
        } else {
          std::cout << "[ ";
          for (size_t j = 0; j < values[i].size(); ++j) {
            if (j > 0)
              std::cout << ", ";
            std::cout << values[i][j];
          }
          std::cout << " ]\n";
        }
      }
      std::cout << "\n";
    }
  }

  // Floats
  {
    auto [keys, values] = params.getKeysAndValues<float>();
    if (!keys.empty()) {
      std::cout << "Float Parameters:\n";
      for (size_t i = 0; i < keys.size(); ++i) {
        std::cout << "  " << std::setw(30) << std::left << keys[i] << " = ";
        if (values[i].size() == 1) {
          std::cout << values[i][0] << "\n";
        } else {
          std::cout << "[ ";
          for (size_t j = 0; j < values[i].size(); ++j) {
            if (j > 0)
              std::cout << ", ";
            std::cout << values[i][j];
          }
          std::cout << " ]\n";
        }
      }
      std::cout << "\n";
    }
  }

  // Doubles
  {
    auto [keys, values] = params.getKeysAndValues<double>();
    if (!keys.empty()) {
      std::cout << "Double Parameters:\n";
      for (size_t i = 0; i < keys.size(); ++i) {
        std::cout << "  " << std::setw(30) << std::left << keys[i] << " = ";
        if (values[i].size() == 1) {
          std::cout << values[i][0] << "\n";
        } else {
          std::cout << "[ ";
          for (size_t j = 0; j < values[i].size(); ++j) {
            if (j > 0)
              std::cout << ", ";
            std::cout << values[i][j];
          }
          std::cout << " ]\n";
        }
      }
      std::cout << "\n";
    }
  }

  // Strings
  {
    auto [keys, values] = params.getKeysAndValues<std::string>();
    if (!keys.empty()) {
      std::cout << "String Parameters:\n";
      for (size_t i = 0; i < keys.size(); ++i) {
        std::cout << "  " << std::setw(30) << std::left << keys[i] << " = ";
        if (values[i].size() == 1) {
          std::cout << '"' << values[i][0] << '"' << "\n";
        } else {
          std::cout << "[ ";
          for (size_t j = 0; j < values[i].size(); ++j) {
            if (j > 0)
              std::cout << ", ";
            std::cout << '"' << values[i][j] << '"';
          }
          std::cout << " ]\n";
        }
      }
      std::cout << "\n";
    }
  }

  std::cout << "=================================\n\n";
}
