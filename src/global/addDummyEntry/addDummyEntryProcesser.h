
#pragma once

#include <JANA/JEvent.h>
#include <JANA/JEventProcessor.h>
#include <podio/ROOTWriter.h>
#include <spdlog/logger.h>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

class addDummyEntryProcesser : public JEventProcessor {

public:
    addDummyEntryProcesser();
    virtual ~addDummyEntryProcesser() = default;

    void Init() override;
    void Process(const std::shared_ptr<const JEvent>& event) override;
    void Finish() override;

    void FindCollectionsToWrite(const std::shared_ptr<const JEvent>& event);

    std::unique_ptr<podio::ROOTWriter> m_writer_addDummy;
    std::mutex m_mutex_addDummy;
    bool m_is_first_event_addDummy            = true;
    bool m_user_included_collections_addDummy = false;
    std::shared_ptr<spdlog::logger> m_log_addDummy;
    bool m_output_include_collections_set_addDummy = false;

    std::string m_output_file_addDummy          = "pythia83kHz_timeslices.root";
    std::string m_output_file_copy_dir_addDummy = "";
    std::set<std::string> m_output_collections_addDummy;         // config. parameter
    std::set<std::string> m_output_exclude_collections_addDummy; // config. parameter
    std::vector<std::string> m_collections_to_write_addDummy;    // derived from above config. parameters
    std::vector<std::string> m_collections_to_print_addDummy;
};

