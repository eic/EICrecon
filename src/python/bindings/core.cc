// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// eicrecon._core: minimal JApplication handle for the Python frontend prototype.
//
// The application is created and run through JANA2's public CLI API (jana::CreateJApplication,
// jana::Execute), so no command-line handling is reimplemented here: argument parsing lives in
// Python (eicrecon/cli.py) and fills a jana::UserOptions.

#include <Evaluator/DD4hepUnits.h>
#include <JANA/CLI/JMain.h>
#include <JANA/JApplication.h>
#include <JANA/Services/JParameterManager.h>
#include <TROOT.h>
#include <edm4eic/unit_system.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "FactoryType.h"

namespace py = pybind11;
using eicrecon::python::FactoryTypeBase;

namespace {

class Application {
public:
  Application(const std::map<std::string, std::string>& params,
              const std::vector<std::string>& event_sources) {
    ROOT::EnableThreadSafety();
    m_options.params       = params;
    m_options.eventSources = event_sources;
    m_app                  = jana::CreateJApplication(m_options);
    japp                   = m_app; // used by e.g. JEventProcessorPODIO
  }

  Application(const Application&)            = delete;
  Application& operator=(const Application&) = delete;

  ~Application() { close(); }

  void close() {
    if (m_app != nullptr) {
      if (japp == m_app) {
        japp = nullptr;
      }
      delete m_app;
      m_app = nullptr;
    }
  }

  JApplication* get() {
    if (m_app == nullptr) {
      throw std::runtime_error("eicrecon._core.Application: already closed");
    }
    return m_app;
  }

  void add_plugin(const std::string& name) { get()->AddPlugin(name); }
  void add_plugin_path(const std::string& path) { get()->AddPluginPath(path); }
  void add_event_source(const std::string& source) { get()->Add(source); }

  void set_parameter(const std::string& key, const std::string& value) {
    get()->GetJParameterManager()->SetParameter(key, value);
  }

  py::object get_parameter(const std::string& key) {
    auto* pm = get()->GetJParameterManager();
    if (!pm->Exists(key)) {
      return py::none();
    }
    return py::str(pm->GetParameterValue<std::string>(key));
  }

  void add_factory(const FactoryTypeBase& type, const std::string& plugin, const std::string& tag,
                   const std::vector<std::string>& inputs, const std::vector<std::string>& outputs,
                   const py::dict& config) {
    // make_generator validates the config and throws (as a Python exception) before anything is
    // handed to JANA
    get()->Add(type.make_generator(get(), plugin, tag, inputs, outputs, config));
  }

  /// Run via jana::Execute. `mode` selects one of JANA's CLI modes.
  int execute(const std::string& mode) {
    auto opts = m_options;
    if (mode == "configs") {
      opts.flags[jana::ShowConfigs] = true;
    } else if (mode == "list-factories") {
      opts.flags[jana::InspectComponent] = true;
    } else if (mode == "list-collections") {
      opts.flags[jana::InspectCollection] = true;
    } else if (mode != "run") {
      throw py::value_error("unknown execute mode '" + mode + "'");
    }
    JApplication* app = get();
    py::gil_scoped_release release;
    return jana::Execute(app, opts);
  }

  uint64_t n_events_processed() { return get()->GetNEventsProcessed(); }

private:
  jana::UserOptions m_options;
  JApplication* m_app = nullptr;
};

} // namespace

PYBIND11_MODULE(_core, m) {
  m.doc() = "EICrecon Python frontend core (prototype)";

  py::class_<FactoryTypeBase, std::shared_ptr<FactoryTypeBase>>(m, "FactoryType")
      .def_property_readonly("name", &FactoryTypeBase::name)
      .def_property_readonly("cpp_type", &FactoryTypeBase::cpp_type)
      .def("config_defaults", &FactoryTypeBase::config_defaults)
      .def("signature", &FactoryTypeBase::signature)
      .def("validate_config", &FactoryTypeBase::validate_config, py::arg("config"),
           py::arg("context") = "")
      .def("__repr__", [](const FactoryTypeBase& t) { return "<FactoryType " + t.name() + ">"; });

  py::class_<Application>(m, "Application")
      .def(py::init<const std::map<std::string, std::string>&, const std::vector<std::string>&>(),
           py::arg("params")        = std::map<std::string, std::string>{},
           py::arg("event_sources") = std::vector<std::string>{})
      .def("add_plugin", &Application::add_plugin)
      .def("add_plugin_path", &Application::add_plugin_path)
      .def("add_event_source", &Application::add_event_source)
      .def("set_parameter", &Application::set_parameter)
      .def("get_parameter", &Application::get_parameter)
      .def("add_factory", &Application::add_factory, py::arg("type"), py::arg("plugin"),
           py::arg("tag"), py::arg("inputs"), py::arg("outputs"), py::arg("config"))
      .def("execute", &Application::execute, py::arg("mode") = "run")
      .def_property_readonly("n_events_processed", &Application::n_events_processed)
      .def("close", &Application::close);

  // Unit systems. Values come from the C++ headers, so Python and C++ wiring agree bit for bit.
  auto dd               = m.def_submodule("dd4hep_units", "dd4hep (TGeo) units: cm = GeV = ns = 1");
  dd.attr("um")         = dd4hep::um;
  dd.attr("mm")         = dd4hep::mm;
  dd.attr("cm")         = dd4hep::cm;
  dd.attr("m")          = dd4hep::m;
  dd.attr("ns")         = dd4hep::ns;
  dd.attr("picosecond") = dd4hep::picosecond;
  dd.attr("s")          = dd4hep::s;
  dd.attr("eV")         = dd4hep::eV;
  dd.attr("keV")        = dd4hep::keV;
  dd.attr("MeV")        = dd4hep::MeV;
  dd.attr("GeV")        = dd4hep::GeV;
  dd.attr("TeV")        = dd4hep::TeV;
  dd.attr("rad")        = dd4hep::rad;
  dd.attr("mrad")       = dd4hep::mrad;
  dd.attr("deg")        = dd4hep::deg;
  dd.attr("tesla")      = dd4hep::tesla;

  auto eu        = m.def_submodule("edm4eic_units", "edm4eic units: mm = GeV = ns = 1");
  eu.attr("um")  = edm4eic::unit::um;
  eu.attr("mm")  = edm4eic::unit::mm;
  eu.attr("cm")  = edm4eic::unit::cm;
  eu.attr("m")   = edm4eic::unit::m;
  eu.attr("ns")  = edm4eic::unit::ns;
  eu.attr("ps")  = edm4eic::unit::ps;
  eu.attr("us")  = edm4eic::unit::us;
  eu.attr("ms")  = edm4eic::unit::ms;
  eu.attr("s")   = edm4eic::unit::s;
  eu.attr("nm")  = edm4eic::unit::nm;
  eu.attr("eV")  = edm4eic::unit::eV;
  eu.attr("keV") = edm4eic::unit::keV;
  eu.attr("MeV") = edm4eic::unit::MeV;
  eu.attr("GeV") = edm4eic::unit::GeV;
}
