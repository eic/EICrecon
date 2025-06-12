#pragma once

#include <JANA/JEvent.h>
#include <JANA/JEventProcessor.h>
#include <TDirectory.h>
#include <spdlog/fwd.h>
#include <memory>

class TrackingTest_processor : public JEventProcessor {
public:
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
  //----------------------------
  // Test imminent tracking output
  void ProcessTrackingResults(const std::shared_ptr<const JEvent>& event);

  void ProcessTrackingMatching(const std::shared_ptr<const JEvent>& event);

  void ProcessGloablMatching(const std::shared_ptr<const JEvent>& event);

  std::shared_ptr<spdlog::logger> m_log;
  TDirectory* m_dir_main{};
};
