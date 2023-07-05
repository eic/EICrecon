
#pragma once

#include <JANA/JEventProcessor.h>
#include <spdlog/spdlog.h>
#include <podio/ROOTFrameWriter.h>
#include <Evaluator/DD4hepUnits.h>


class JEventProcessorPODIO : public JEventProcessor {

public:

    JEventProcessorPODIO();
    virtual ~JEventProcessorPODIO() = default;

    void Init() override;
    void Process(const std::shared_ptr<const JEvent>& event) override;
    void Finish() override;

    void FindCollectionsToWrite(const std::shared_ptr<const JEvent>& event);

    std::unique_ptr<podio::ROOTFrameWriter> m_writer;
    std::mutex m_mutex;
    bool m_is_first_event = true;
    bool m_user_included_collections = false;
    std::shared_ptr<spdlog::logger> m_log;

    std::string m_output_file = "podio_output.root";
    std::string m_output_file_copy_dir = "";
    std::set<std::string> m_output_include_collections;  // config. parameter
    std::set<std::string> m_output_exclude_collections;  // config. parameter
    std::vector<std::string> m_collections_to_write;  // derived from above config. parameters
    std::vector<std::string> m_collections_to_print;

    // Beam configuration parameters
    float m_electron_energy = 10*dd4hep::GeV;
    int   m_electron_pdg    = 11;
    float m_ion_energy      = 100*dd4hep::GeV;
    int   m_ion_pdg         = 2212;     
    float m_crossing_angle  = -0.025*dd4hep::rad;

};
