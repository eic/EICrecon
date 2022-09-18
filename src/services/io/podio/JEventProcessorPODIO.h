
#ifndef _EICRootWriterSimple_h_
#define _EICRootWriterSimple_h_

#include <JANA/JEventProcessor.h>
#include <spdlog/spdlog.h>

#include <services/io/podio/RootWriter.h>
#include <services/io/podio/EventStore.h>


class JEventProcessorPODIO : public JEventProcessor {

public:

    JEventProcessorPODIO();
    virtual ~JEventProcessorPODIO() = default;

    void Init() override;
    void Process(const std::shared_ptr<const JEvent>& event) override;
    void Finish() override;

    eic::EventStore *m_store=nullptr;
    std::shared_ptr<eic::ROOTWriter> m_writer;

    std::mutex m_mutex;
    bool m_is_first_event = true;

    std::shared_ptr<spdlog::logger> m_log;

    std::string m_output_file = "podio_output.root";
    std::string m_output_file_copy_dir = "";
    std::set<std::string> m_output_include_collections;  // config. parameter
    std::set<std::string> m_output_exclude_collections;  // config. parameter
    std::set<std::string> m_collections_to_write;        // derived from above config. parameters

};


#endif // _EICRootWriterSimple_h_

