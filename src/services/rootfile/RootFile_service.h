#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <mutex>

#include <JANA/JApplicationFwd.h>
#include "services/log/Log_service.h"
#include <JANA/Services/JGlobalRootLock.h>
#include <JANA/Services/JServiceLocator.h>

#include <TFile.h>

/**
 * This Service centralizes creation of a root file for histograms
 */
class RootFile_service : public JService {
public:
  explicit RootFile_service(JApplication* app) : m_app(app) {}
  ~RootFile_service() override { CloseHistFile(); }

  void acquire_services(JServiceLocator* /* locayor */) override {
    auto log_service = m_app->GetService<Log_service>();
    m_log            = log_service->logger("RootFile");
  }

  /// This will return a pointer to the top-level directory of the
  /// common output root file for histograms. If create_if_needed
  /// is true and the root file has not already been created, then
  /// one will be created. If create_if_needed is false, the pointer
  /// to the existing file will be returned or nullptr if it does
  /// not already exist.
  ///
  /// NOTE: This should only be called by a thread already holding
  /// the global root lock. The root lock will already be held if
  /// calling from JEventProcessorSequentialRoot. For pretty much
  /// everyplace else, the lock should be acquired manually.
  /// e.g.
  ///
  ///    auto rootfile_service = japp->GetService<RootFile_service>();
  ///    auto globalRootLock = japp->GetService<JGlobalRootLock>();
  ///    globalRootLock->acquire_write_lock();
  ///    auto rootfile = rootfile_service->GetHistFile();
  ///    globalWriteLock->release_lock();
  ///
  /// \param create_if_needed create file if not already created
  /// \return
  TDirectory* GetHistFile(bool create_if_needed = true) {

    if (create_if_needed) {
      std::call_once(init_flag, &RootFile_service::CreateHistFile, this);
    }
    return m_histfile;
  }

  /// Close the histogram file. If no histogram file was opened,
  /// then this does nothing.
  ///
  /// This should generally never be called by anything other than
  /// the destructor so that it is
  /// automatically closed when the service is destructed at
  /// the end of processing. This is only here for use in
  /// execptional circumstances like the program is suffering
  /// a fatal crash and we want to try and save the work by
  /// closing the file cleanly.
  void CloseHistFile() {
    if (m_histfile) {
      std::string filename = m_histfile->GetName();
      m_histfile->Write();
      delete m_histfile;
      m_log->info("Closed user histogram file: {}", filename);
    }
    m_histfile = nullptr;
  }

private:
  /// Create the output rootfile. This will be called only once
  /// which will happen the first time GetHistFile is called.
  void CreateHistFile() {
    // Get root file name
    std::string filename = "eicrecon.root";
    m_app->SetDefaultParameter("histsfile", filename,
                               "Name of root file to be created for plugin histograms/trees");
    if (!m_histfile) {
      try {
        m_histfile = new TFile(filename.c_str(), "RECREATE", "user histograms/trees");
        m_log->info("Created file: {} for user histograms", filename);
      } catch (std::exception& ex) {
        m_log->error("Problem opening root file for histograms: {}", ex.what());
        throw ex;
      }
    }
  }

  RootFile_service() = default;

  JApplication* m_app = nullptr;
  std::shared_ptr<spdlog::logger> m_log;
  TFile* m_histfile = nullptr;
  std::once_flag init_flag;
};
