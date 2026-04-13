#pragma once

#include <JANA/JEventSourceGeneratorT.h>
#include <atomic>
#include <condition_variable>
#include <memory>
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
