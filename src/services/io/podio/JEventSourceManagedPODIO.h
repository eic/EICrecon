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

  void SetCurrentFile(const std::string& input_file, uint64_t nskip = 0, uint64_t nevents = 0);
  bool IsFileProcessingComplete() const { return m_file_processing_complete.load(); }
  std::size_t GetNeventsInFile() const { return m_effective_nevents; }

  void ResetReader();

private:
  // File management for managed mode
  std::string m_current_input_file;
  std::atomic<bool> m_file_available{false};
  std::atomic<bool> m_file_processing_complete{false};
  std::atomic<bool> m_closing{false};

  // Per-request skip/limit parameters
  uint64_t m_nskip = 0;    // Number of events to skip from start of file
  uint64_t m_nevents = 0;  // Max events to process (0 = all remaining after skip)

  // Effective number of events to process for the current file
  // (accounting for nskip and nevents)
  std::size_t m_effective_nevents = 0;

  // Synchronization
  std::mutex m_file_mutex;
  std::condition_variable m_file_cv;
};

template <> double JEventSourceGeneratorT<JEventSourceManagedPODIO>::CheckOpenable(std::string);
