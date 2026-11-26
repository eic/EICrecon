// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026, ePIC Collaboration

#pragma once

#include <JANA/JApplicationFwd.h>
#include <JANA/JEventSource.h>
#include <JANA/JEventSourceGenerator.h>
#include <podio/Frame.h>
#include <podio/Reader.h>
#include <spdlog/logger.h>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

/// JANA2 event source for podio files.
///
/// Files are opened with podio's generic `podio::makeReader`, so both the TTree
/// and the RNTuple backends are supported. Every entry of the "events" category
/// is turned into one JEvent. The serialized `Emit()` only reads the entry and
/// inserts the `podio::Frame`; `ProcessParallel()` then exposes each collection
/// it holds through a `JPodioDatabundle` named after the collection, so
/// unpacking happens on the worker threads and downstream factories cannot
/// tell whether a collection was read from file or produced in this job.
///
/// The other frame categories (e.g. run-level metadata) are not emitted as
/// events; `JEventProcessorPODIO` copies them to the output file via
/// `getAvailableCategories()`, `getEntries()` and `getFrame()`.
///
/// Parameters:
///  - `podio:run_forever`      restart from the first entry when the file is exhausted
///  - `podio:print_type_table` print collection names and types when the file is opened
class JEventSourcePODIO : public JEventSource {
public:
  JEventSourcePODIO(std::string resource_name, JApplication* app);
  ~JEventSourcePODIO() override = default;

  static std::string GetDescription();

  // JEventSource callbacks (CallbackStyle::ExpertMode)
  void Open() override;
  void Close() override;
  Result Emit(JEvent& event) override;
  void ProcessParallel(JEvent& event) const override;
  std::pair<Result, std::size_t> Skip(JEvent& event, std::size_t events_to_skip) override;

  // Read access to all frame categories of the open file
  std::vector<std::string_view> getAvailableCategories() const;
  std::size_t getEntries(const std::string& category) const;
  podio::Frame getFrame(const std::string& category, std::size_t index) const;

  void PrintCollectionTypeTable();

protected:
  static constexpr const char* k_event_category = "events";

  std::shared_ptr<spdlog::logger> m_log;
  std::unique_ptr<podio::Reader> m_reader;

  std::size_t m_event_entries = 0; ///< number of entries in the "events" category
  std::size_t m_next_entry    = 0; ///< index of the next "events" entry to emit

  bool m_run_forever        = false;
  bool m_print_type_table   = false;
  bool m_trust_event_header = true; ///< cleared after the first malformed EventHeader

private:
  bool rewindIfExhausted();
  void applyEventHeader(const podio::Frame& frame, JEvent& event);
};

/// Generator that lets JANA pick JEventSourcePODIO for podio files.
class JEventSourcePODIOGenerator : public JEventSourceGenerator {
public:
  std::string GetType() const override;
  std::string GetDescription() const override;
  JEventSource* MakeJEventSource(std::string resource_name) override;
  double CheckOpenable(std::string resource_name) override;
};
