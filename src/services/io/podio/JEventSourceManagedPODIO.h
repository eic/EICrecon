#pragma once

#include <JANA/JApplicationFwd.h>
#include <JANA/JEventSourceGeneratorT.h>
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <string>

#include "JEventSourcePODIO.h"

class JEventSourceManagedPODIO : public JEventSourcePODIO {

public:
  JEventSourceManagedPODIO(std::string resource_name, JApplication* app);
  virtual ~JEventSourceManagedPODIO();

  void Open() override;
  void Close() override;
  Result Emit(JEvent& event) override;

  static std::string GetDescription();

  void SetCurrentFile(const std::string& input_file);
  bool IsFileProcessingComplete() const { return m_file_processing_complete.load(); }
  std::size_t GetNeventsInFile() const { return Nevents_in_file; }

private:
  // File management for managed mode
  std::string m_current_input_file;
  std::atomic<bool> m_file_available{false};
  std::atomic<bool> m_file_processing_complete{false};

  // Synchronization
  std::mutex m_file_mutex;
  std::condition_variable m_file_cv;
};

template <> double JEventSourceGeneratorT<JEventSourceManagedPODIO>::CheckOpenable(std::string);
