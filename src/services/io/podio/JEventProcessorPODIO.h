
#pragma once

#include <JANA/JEvent.h>
#include <JANA/JEventProcessor.h>
#include <podio/podioVersion.h>
#if podio_VERSION >= PODIO_VERSION(0, 99, 0)
#include <podio/ROOTWriter.h>
#else
#include <podio/ROOTFrameWriter.h>
#endif
#include <spdlog/logger.h>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>


class JEventProcessorPODIO : public JEventProcessor {

public:

    JEventProcessorPODIO();
    virtual ~JEventProcessorPODIO() = default;

    void Init() override;
    void Process(const std::shared_ptr<const JEvent>& event) override;
    void Finish() override;

    void FindCollectionsToWrite(const std::shared_ptr<const JEvent>& event);

#if podio_VERSION >= PODIO_VERSION(0, 99, 0)
    std::unique_ptr<podio::ROOTWriter> m_writer;
#else
    std::unique_ptr<podio::ROOTFrameWriter> m_writer;
#endif
    std::mutex m_mutex;
    bool m_is_first_event = true;
    bool m_user_included_collections = false;
    std::shared_ptr<spdlog::logger> m_log;
    bool m_output_include_collections_set = false;

    std::string m_output_file = "podio_output.root";
    std::string m_output_file_copy_dir = "";
    std::set<std::string> m_output_collections;  // config. parameter
    std::set<std::string> m_output_exclude_collections;  // config. parameter
    std::vector<std::string> m_collections_to_write;  // derived from above config. parameters
    std::vector<std::string> m_collections_to_print;

};
