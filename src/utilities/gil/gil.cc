#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#include <fmt/core.h>

#include <TROOT.h>
#include <TInterpreter.h>
#include <TInterpreterValue.h>

#include <edm4hep/RawTimeSeriesCollection.h>
#include <podio/Frame.h>
#include <podio/ROOTWriter.h>

std::mutex mutex;

void task1(std::size_t i) {
  std::lock_guard<std::mutex> lock(mutex);

  std::string func_name = fmt::format("f{}", i);
  std::ostringstream sstr;
  sstr << "double " << func_name << "(double params[]){";
  sstr << "double row_1 = params[0];";
  sstr << "double row_2 = params[1];";
  sstr << "double column_1 = params[2];";
  sstr << "double column_2 = params[3];";
  sstr << "return max(abs(row_1 - row_2), abs(column_1 - column_2)) == 1;";
  sstr << "}";

  TInterpreter* interp = TInterpreter::Instance();
  interp->ProcessLine(sstr.str().c_str());
  std::unique_ptr<TInterpreterValue> func_val{gInterpreter->MakeInterpreterValue()};
  interp->Evaluate(func_name.c_str(), *func_val);
  typedef double (*func_t)(double params[]);
  func_t func = ((func_t)(func_val->GetAsPointer()));

  double params[] = {0, 1, 0, 1};
  std::cout << func_name << " = " << func(params) << std::endl;
}

void task2(std::size_t i) {
  static auto m_writer = std::make_unique<podio::ROOTWriter>("podioOutput.root");

  podio::Frame event{};

  // write some RawTimeSeries:
  auto tpchs = edm4hep::RawTimeSeriesCollection();
  int ntpch  = 5;
  for (int j = 0; j < ntpch; ++j) {
    auto tpch1 = tpchs.create();
    tpch1.setCellID(0xabadcaffee);
    tpch1.setTime(j * 0.3f);
    tpch1.setCharge(j * 2.f);

    auto tpch2 = tpchs.create();
    tpch2.setCellID(0xcaffeebabe);
    tpch2.setTime(j * 0.3f);
    tpch2.setCharge(-j * 2.f);
  }

  std::cout << "\n collection: "
            << "Time Projection Chamber Hits"
            << " of type " << tpchs.getValueTypeName() << "\n\n"
            << tpchs << std::endl;
  event.put(std::move(tpchs), "TPCHits");

  m_writer->writeFrame(event, "events");
}

int main(int argc, char** argv) {
  ROOT::EnableThreadSafety();

  std::vector<std::thread> threads;
  for (std::size_t i = 0; i < argc; ++i) {
    threads.emplace_back(task1, i);
  }
  for (std::size_t i = 0; i < argc; ++i) {
    threads.emplace_back(task2, i);
  }

  for (auto& t : threads) {
    t.join();
  }

  return 0;
}
