#ifndef __RootFile_service_h__
#define __RootFile_service_h__


#include <iostream>
#include <vector>
#include <string>

#include <JANA/JApplication.h>
#include <services/log/Log_service.h>
#include <JANA/Services/JGlobalRootLock.h>
#include <JANA/Services/JServiceLocator.h>

#include <TFile.h>

/**
 * This Service centralizes creation of a root file for histograms
 */
class RootFile_service : public JService
{
public:
    explicit RootFile_service(JApplication *app ):m_app(app){}
    ~RootFile_service() override { delete m_histfile; }

    void acquire_services(JServiceLocator *locater) override {
        m_GlobalWriteLock = locater->get<JGlobalRootLock>();
        auto log_service = m_app->GetService<Log_service>();
        m_log = log_service->logger("RootFile");
    }

    /// This will return a pointer to the top-level directory of the
    /// common output root file for histograms. If create_if_needed
    /// is true and the root file has not already been created, then
    /// one will be created. If create_if_needed is false, the pointer
    /// to the existing file will be returned or nullptr if it does
    /// not already exist.
    TDirectory* GetHistFile(bool create_if_needed=true){

        if( create_if_needed ) {

            // Get root file name
            std::string filename = "eicrecon.root";
            m_app->SetDefaultParameter("histsfile", filename,
                                     "Name of root file to be created for plugin histograms/trees");

            m_GlobalWriteLock->acquire_write_lock(); // TODO: JANA needs this to allow lock_guard style usage
            if (!m_histfile) {
                try {
                    m_histfile = new TFile(filename.c_str(), "RECREATE", "user histograms/trees");
                    m_log->info("Created file: {} for user histograms", filename);
                } catch (std::exception &ex) {
                    m_GlobalWriteLock->release_lock();
                    m_log->error("Problem opening root file for histograms: {}", ex.what());
                    throw ex;
                }
            }

            m_GlobalWriteLock->release_lock();
        }
        return m_histfile;
    }

    /// Close the histogram file. If no histogram file was opened,
    /// then this does nothing.
    ///
    /// This should generally never be called. The file will be
    /// automatically closed when the service is destructed at
    /// the end of processing. This is only here for use in
    /// execptional circumstances like the program is suffering
    /// a fatal crash and we want to try and save the work by
    /// closing the file cleanly.
    ///
    /// n.b. If GetHistFile() is called *after* this, then the file
    /// will be created again, overwriting the previous one!
    void CloseHistFile(){
        if( m_histfile){
            m_GlobalWriteLock->acquire_write_lock(); // Do we really want this here?
            delete m_histfile;
            m_GlobalWriteLock->release_lock();
        }
        m_histfile = nullptr;
    }

private:

    RootFile_service()=default;

    JApplication *m_app=nullptr;
    std::shared_ptr<JGlobalRootLock> m_GlobalWriteLock;
    std::shared_ptr<spdlog::logger> m_log;
    TFile *m_histfile = nullptr;
};

#endif // __RootFile_service_h__
