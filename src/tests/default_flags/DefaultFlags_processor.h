#ifndef EICRECON_OCCUPANCY_ANALYSIS_H
#define EICRECON_OCCUPANCY_ANALYSIS_H

#include <TH1F.h>
#include <TH3F.h>
#include <TH2F.h>

#include <JANA/JEventProcessor.h>

class JEvent;
class JApplication;

class DefaultFlags_processor:public JEventProcessor
{
public:
    explicit DefaultFlags_processor(JApplication *);
    ~DefaultFlags_processor() override = default;

    //----------------------------
    // Init
    //
    // This is called once before the first call to the Process method
    // below. You may, for example, want to open an output file here.
    // Only one thread will call this.
    void Init() override;

    //----------------------------
    // Process
    //
    // This is called for every event. Multiple threads may call this
    // simultaneously. If you write something to an output file here
    // then make sure to protect it with a mutex or similar mechanism.
    // Minimize what is done while locked since that directly affects
    // the multi-threaded performance.
    void Process(const std::shared_ptr<const JEvent>& event) override;

    //----------------------------
    // Finish
    //
    // This is called once after all events have been processed. You may,
    // for example, want to close an output file here.
    // Only one thread will call this.
    void Finish() override;

private:
    std::string m_python_file_name = "";
    std::string m_markdown_file_name = "";
    std::vector<std::string> m_valuable_subsystems = {
            "B0TRK",
            "BEMC",
            "BTRK",
            "BVTX",
            "ECGEM",
            "ECTRK",
            "EEMC",
            "FOFFMTRK",
            "HCAL",
            "MPGD",
            "RPOTS",
            "ZDC",
            "Tracking",
            "Reco",
            "Digi",
            "Calorimetry"
    };
};

#endif //EICRECON_OCCUPANCY_ANALYSIS_H
