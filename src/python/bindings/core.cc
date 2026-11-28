// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 Wouter Deconinck
//
// eicrecon._core: JApplication handle for the Python frontend.
//
// The application is created and run through JANA2's public CLI API (jana::CreateJApplication,
// jana::Execute). Argument parsing lives in Python (eicrecon/cli.py), which also prints the
// parameter and factory tables from the data returned by parameters() and factories().

#include <DD4hep/Detector.h>
#include <DD4hep/Readout.h>
#include <DD4hep/Segmentations.h>
#include <Evaluator/DD4hepUnits.h>
#include <JANA/CLI/JBenchmarker.h>
#include <JANA/CLI/JMain.h>
#include <JANA/Components/JComponentSummary.h>
#include <JANA/JApplication.h>
#include <JANA/JApplicationFwd.h>
#include <JANA/JEventSource.h>
#include <JANA/JException.h>
#include <JANA/JFactoryGenerator.h>
#include <JANA/JVersion.h>
#include <JANA/Services/JComponentManager.h>
#include <JANA/Services/JParameterManager.h>
#include <TROOT.h>
#include <edm4eic/unit_system.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep
#include <cstdint>
#include <gsl/pointers>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <typeinfo>
#include <utility>
#include <variant>
#include <vector>

#include "ConfigConversion.h"
#include "FactoryType.h"
#include "extensions/jana/JComponentManager_compat.h"
#include "extensions/jana/JOmniFactoryObserver.h"
#include "services/geometry/dd4hep/DD4hep_service.h"

namespace py = pybind11;
using eicrecon::python::CppFunction;
using eicrecon::python::factory_type_registry;
using eicrecon::python::FactoryTypeBase;

namespace {

/// Context handed to a deferred graph section while JANA generates factories: services
/// (geometry, parameters) are available, and factories added here join the graph.
class DeferredContext {
public:
  DeferredContext(JApplication* app, std::string plugin,
                  std::vector<std::unique_ptr<JFactoryGenerator>>* generators)
      : m_app(app), m_plugin(std::move(plugin)), m_generators(generators) {}

  std::string plugin() const { return m_plugin; }
  eicrecon::python::AppHandle app() const { return {m_app}; }

  void add_factory(const FactoryTypeBase& type, const std::string& tag,
                   const std::vector<std::string>& inputs, const std::vector<std::string>& outputs,
                   const py::dict& config) {
    m_generators->emplace_back(type.make_generator(m_app, m_plugin, tag, inputs, outputs, config));
  }

  const dd4hep::Detector* detector() { return m_app->GetService<DD4hep_service>()->detector(); }

  /// Integer constant from the geometry, or None if it is not defined
  py::object constant_int(const std::string& name) {
    try {
      return py::int_(detector()->constant<int>(name));
    } catch (const std::runtime_error&) {
      return py::none();
    }
  }
  /// Floating-point constant from the geometry (dd4hep units), or None if it is not defined
  py::object constant_double(const std::string& name) {
    try {
      return py::float_(detector()->constant<double>(name));
    } catch (const std::runtime_error&) {
      return py::none();
    }
  }
  /// Segmentation type of a readout (e.g. "CartesianGridXY"), or None if there is no such readout
  py::object readout_segmentation_type(const std::string& readout) {
    try {
      return py::str(detector()->readout(readout).segmentation().type());
    } catch (...) {
      return py::none();
    }
  }
  /// Register a string parameter with a default and return its value (as SetDefaultParameter)
  std::string parameter(const std::string& key, std::string value, const std::string& description) {
    m_app->SetDefaultParameter(key, value, description);
    return value;
  }

private:
  JApplication* m_app;
  std::string m_plugin;
  std::vector<std::unique_ptr<JFactoryGenerator>>* m_generators;
};

/// A graph section defined by a Python function that needs services (e.g. geometry constants).
/// The function runs once, on the first GenerateFactories call (inside JApplication::Initialize,
/// when services are available); the factories it adds are generated from then on.
class PyDeferredGenerator : public JFactoryGenerator {
public:
  PyDeferredGenerator(JApplication* app, std::string plugin, py::function fn)
      : m_app(app), m_plugin(std::move(plugin)), m_fn(std::move(fn)) {}

  PyDeferredGenerator(const PyDeferredGenerator&)            = delete;
  PyDeferredGenerator& operator=(const PyDeferredGenerator&) = delete;
  PyDeferredGenerator(PyDeferredGenerator&&)                 = delete;
  PyDeferredGenerator& operator=(PyDeferredGenerator&&)      = delete;

  ~PyDeferredGenerator() override {
    // Release the Python function with the GIL held; a destructor must not throw, so if that
    // fails (e.g. the interpreter is already finalized), leak the reference instead
    try {
      py::gil_scoped_acquire gil;
      m_fn = py::function();
    } catch (...) {
      m_fn.release();
    }
  }

  void GenerateFactories(JFactorySet* factory_set) override {
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      if (!m_done) {
        m_done = true;
        py::gil_scoped_acquire gil;
        DeferredContext ctx(m_app, m_plugin, &m_generators);
        try {
          m_fn(py::cast(&ctx, py::return_value_policy::reference));
        } catch (py::error_already_set& e) {
          m_error = "graph section of plugin '" + m_plugin + "' failed: " + e.what();
        }
      }
    }
    if (!m_error.empty()) {
      throw JException(m_error);
    }
    for (auto& generator : m_generators) {
      generator->GenerateFactories(factory_set);
    }
  }

private:
  JApplication* m_app;
  std::string m_plugin;
  py::function m_fn;
  std::mutex m_mutex;
  bool m_done = false;
  std::string m_error;
  std::vector<std::unique_ptr<JFactoryGenerator>> m_generators;
};

/// Records every wiring that JOmniFactoryGeneratorT instantiates (C++ plugins and Python alike)
class WiringCapture {
public:
  void start() {
    m_observer = [this](const eicrecon::JOmniFactoryWiringRecord& r) { record(r); };
    eicrecon::g_omnifactory_observer = &m_observer;
  }
  void stop() {
    if (eicrecon::g_omnifactory_observer == &m_observer) {
      eicrecon::g_omnifactory_observer = nullptr;
    }
  }
  WiringCapture()                                = default;
  WiringCapture(const WiringCapture&)            = delete;
  WiringCapture& operator=(const WiringCapture&) = delete;
  WiringCapture(WiringCapture&&)                 = delete;
  WiringCapture& operator=(WiringCapture&&)      = delete;
  ~WiringCapture() { stop(); }

  py::list records() {
    std::lock_guard<std::mutex> lock(m_mutex);
    py::list out;
    for (const auto& r : m_records) {
      out.append(r);
    }
    return out;
  }

private:
  void record(const eicrecon::JOmniFactoryWiringRecord& r) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto key            = r.plugin + ":" + r.tag;
    auto [it, inserted] = m_generators.emplace(key, r.generator);
    if (!inserted) {
      if (it->second != r.generator && m_duplicates.insert(key).second) {
        // a second generator wires the same plugin:tag (e.g. C++ plugin and Python alike)
        py::gil_scoped_acquire gil;
        for (auto& d : m_records) {
          if (py::cast<std::string>(d["plugin"]) == r.plugin &&
              py::cast<std::string>(d["tag"]) == r.tag) {
            d["duplicate"] = true;
          }
        }
      }
      return; // GenerateFactories runs once per factory set
    }
    py::gil_scoped_acquire gil;
    py::dict d;
    d["plugin"]   = r.plugin;
    d["tag"]      = r.tag;
    d["cpp_type"] = eicrecon::python::detail::demangle(r.factory_type->name());
    d["inputs"]   = *r.inputs;
    d["outputs"]  = *r.outputs;
    std::shared_ptr<FactoryTypeBase> type;
    {
      auto& registry = factory_type_registry();
      std::lock_guard<std::mutex> rlock(registry.mutex);
      auto it = registry.types.find(r.factory_type->name());
      if (it != registry.types.end()) {
        type = it->second;
      }
    }
    if (type) {
      d["type"]   = type->name();
      d["config"] = type->config_to_dict(r.config);
    } else {
      d["type"]   = py::none();
      d["config"] = py::none();
    }
    m_records.push_back(std::move(d));
  }

  eicrecon::JOmniFactoryObserver m_observer;
  std::mutex m_mutex;
  std::map<std::string, const void*> m_generators;
  std::set<std::string> m_duplicates;
  std::vector<py::dict> m_records;
};

class Application {
public:
  Application(const std::map<std::string, std::string>& params,
              const std::vector<std::string>& event_sources, const std::string& load_config_file,
              const std::vector<std::string>& plugin_paths) {
    ROOT::EnableThreadSafety();
    m_options.params       = params;
    m_options.eventSources = event_sources;
    if (!load_config_file.empty()) {
      m_options.flags[jana::LoadConfigs] = true;
      m_options.load_config_file         = load_config_file;
    }
    m_app = jana::CreateJApplication(m_options);
    japp  = m_app; // used by e.g. JEventProcessorPODIO
    for (const auto& path : plugin_paths) {
      m_app->AddPluginPath(path);
    }
  }

  Application(const Application&)            = delete;
  Application& operator=(const Application&) = delete;
  Application(Application&&)                 = delete;
  Application& operator=(Application&&)      = delete;

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

  /// Set a parameter only if neither the command line nor a config file set it
  void set_default_parameter(const std::string& key, const std::string& value) {
    auto* pm = get()->GetJParameterManager();
    if (pm->FindParameter(key) == nullptr) {
      pm->SetParameter(key, value);
    }
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

  void add_deferred(const std::string& plugin, py::function fn) {
    get()->Add(new PyDeferredGenerator(get(), plugin, std::move(fn)));
  }

  void initialize() {
    JApplication* app = get();
    py::gil_scoped_release release;
    app->Initialize();
  }

  /// All parameters as (key, value, description), sorted by key. Call after initialize().
  std::vector<std::tuple<std::string, std::string, std::string>> parameters() {
    std::vector<std::tuple<std::string, std::string, std::string>> result;
    for (const auto& [key, p] : get()->GetJParameterManager()->GetAllParameters()) {
      result.emplace_back(key, p->GetValue(), p->GetDescription());
    }
    return result;
  }

  /// All factories as (plugin, object name, tag). Call after initialize().
  std::vector<std::tuple<std::string, std::string, std::string>> factories() {
    std::vector<std::tuple<std::string, std::string, std::string>> result;
    for (const auto& factory : get()->GetComponentSummary().factories) {
      result.emplace_back(factory.plugin_name, factory.object_name, factory.factory_tag);
    }
    return result;
  }

  /// Open the podio event sources so that they print their collection type table
  /// (-Ppodio:print_type_table=1). Call after initialize().
  void open_podio_sources() {
    auto component_manager = get()->GetService<JComponentManager>();
    for (auto* source : eicrecon::jana_compat::GetEventSources(component_manager)) {
      if (source->GetPluginName().find("podio") != std::string::npos) {
        source->DoOpen();
      }
    }
  }

  /// Register the benchmarking parameters, so that they are listed with the other parameters
  /// (eicrecon -c -b). Call after initialize().
  void register_benchmark_parameters() { JBenchmarker benchmarker(get()); }

  /// Run through jana::Execute: mode is "run", "benchmark" or "dump-configs"
  int execute(const std::string& mode, const std::string& dump_config_file) {
    auto opts = m_options;
    if (mode == "benchmark") {
      opts.flags[jana::Benchmark] = true;
    } else if (mode == "dump-configs") {
      opts.flags[jana::DumpConfigs] = true;
      opts.dump_config_file         = dump_config_file;
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
  m.doc() = "EICrecon Python frontend core";

#ifdef EICRECON_APP_VERSION_STR
  m.attr("eicrecon_version") = EICRECON_APP_VERSION_STR;
#else
  m.attr("eicrecon_version") = "unknown";
#endif
  m.attr("jana_version") = JVersion::GetVersion();

  py::class_<FactoryTypeBase, std::shared_ptr<FactoryTypeBase>>(m, "FactoryType")
      .def_property_readonly("name", &FactoryTypeBase::name)
      .def_property_readonly("cpp_type", &FactoryTypeBase::cpp_type)
      .def("config_defaults", &FactoryTypeBase::config_defaults)
      .def("signature", &FactoryTypeBase::signature)
      .def("validate_config", &FactoryTypeBase::validate_config, py::arg("config"),
           py::arg("context") = "")
      .def("__repr__", [](const FactoryTypeBase& t) { return "<FactoryType " + t.name() + ">"; });

  py::class_<CppFunction>(m, "CppFunction")
      .def_readonly("type_name", &CppFunction::type_name)
      .def_readonly("description", &CppFunction::description)
      .def("__repr__", [](const CppFunction& f) {
        return "<CppFunction " + f.type_name + " (" + f.description + ")>";
      });

  py::class_<eicrecon::python::AppHandle>(m, "AppHandle")
      .def("__repr__", [](const eicrecon::python::AppHandle&) { return "<AppHandle>"; });

  py::class_<DeferredContext>(m, "DeferredContext")
      .def_property_readonly("plugin", &DeferredContext::plugin)
      .def_property_readonly("app", &DeferredContext::app)
      .def("add_factory", &DeferredContext::add_factory, py::arg("type"), py::arg("tag"),
           py::arg("inputs"), py::arg("outputs"), py::arg("config"))
      .def("constant_int", &DeferredContext::constant_int)
      .def("constant_double", &DeferredContext::constant_double)
      .def("readout_segmentation_type", &DeferredContext::readout_segmentation_type)
      .def("parameter", &DeferredContext::parameter, py::arg("key"), py::arg("default"),
           py::arg("description") = "");

  py::class_<WiringCapture>(m, "WiringCapture")
      .def(py::init<>())
      .def("start", &WiringCapture::start)
      .def("stop", &WiringCapture::stop)
      .def("records", &WiringCapture::records);

  py::class_<Application>(m, "Application")
      .def(py::init<const std::map<std::string, std::string>&, const std::vector<std::string>&,
                    const std::string&, const std::vector<std::string>&>(),
           py::arg("params")           = std::map<std::string, std::string>{},
           py::arg("event_sources")    = std::vector<std::string>{},
           py::arg("load_config_file") = std::string{},
           py::arg("plugin_paths")     = std::vector<std::string>{})
      .def("add_plugin", &Application::add_plugin)
      .def("add_plugin_path", &Application::add_plugin_path)
      .def("add_event_source", &Application::add_event_source)
      .def("set_parameter", &Application::set_parameter)
      .def("set_default_parameter", &Application::set_default_parameter)
      .def("get_parameter", &Application::get_parameter)
      .def("add_factory", &Application::add_factory, py::arg("type"), py::arg("plugin"),
           py::arg("tag"), py::arg("inputs"), py::arg("outputs"), py::arg("config"))
      .def("add_deferred", &Application::add_deferred, py::arg("plugin"), py::arg("function"))
      .def("initialize", &Application::initialize)
      .def("parameters", &Application::parameters)
      .def("factories", &Application::factories)
      .def("open_podio_sources", &Application::open_podio_sources)
      .def("register_benchmark_parameters", &Application::register_benchmark_parameters)
      .def("execute", &Application::execute, py::arg("mode") = "run",
           py::arg("dump_config_file") = "jana.config")
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
