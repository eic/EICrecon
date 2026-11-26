# Python frontend for EICrecon

Status: **phase 2** (see section 5). `eicrecon-py` is built by default next to `eicrecon`; every
plugin's factory wiring is defined in Python (`eicrecon.plugins`) and produces identical output.
The C++ frontend and `InitPlugin` functions still exist.

## Goal

Today an EICrecon job is defined in two places, both in C++:

1. The `eicrecon` executable (`src/utilities/eicrecon/eicrecon.cc`, `eicrecon_cli.cc`) builds a
   `JApplication` from a hard-coded default plugin list and its own command-line parsing. Part of
   that parsing was copied from JANA2's `JMain.cc` without the JSA notice.
2. Every detector and global plugin has an `InitPlugin` (`src/detectors/*/*.cc`,
   `src/global/**/*.cc`) that registers `JOmniFactoryGeneratorT<T>` instances with a tag, input
   and output collection names, and a config struct.

The target end state:

* The `eicrecon` executable is replaced by a Python script (`eicrecon-py`, later `eicrecon`).
* The `InitPlugin` wiring files become Python modules that define the algorithm graph. Users can
  import them, call them as building blocks, copy them, or modify them, all without recompiling.
* Algorithms and `JOmniFactory` classes stay in C++. Python only does the wiring.

## 1. Survey

### 1.1 What JANA2 offers for Python

| Component | What it is | Usable for this? |
|---|---|---|
| `src/python/plugins/janapy` | A JANA plugin that embeds an interpreter and runs `jana.py` from `InitPlugin`, on a detached thread. | No. The interpreter starts after the `JApplication` exists, so Python cannot own the job. The API is a flat set of global functions over a static `JApplication*`. |
| `src/python/modules/jana` (`jana.so`) | A pybind11 extension module with the same global-function API (`AddPlugin`, `SetParameterValue`, `Run`, `AddProcessor`). It creates a `JApplication` at **import** time. | No as is. It exposes nothing about factories or wiring. In the current `eic_xl:nightly` image, `import jana` also fails with `undefined symbol: md5_finish`. |
| `JEventProcessorPY` | Lets you write event processors in Python. | Not needed. Our algorithms stay in C++, and per-event Python would add a GIL bottleneck. |
| `JWiringService` + `JWiredFactoryGeneratorT` | TOML wiring files (`jana:wiring_file`): each entry gives `plugin_name`, `type_name`, `prefix`, input and output names, and `configs` (string→string). A plugin registers a factory *type* once, and the TOML file creates the instances. | A useful pattern: type registration separate from instantiation. But configs are `map<string,string>` applied through declared parameters (see 1.2), and it works with JANA's `JOmniFactory`, not EICrecon's fork in `src/extensions/jana`. |
| `jana::CreateJApplication(UserOptions&)`, `jana::Execute(app, UserOptions&)` (`JANA/CLI/JMain.h`) | JANA's public entry-point API: it builds the parameter manager and application, then runs or inspects (`ShowConfigs`, `InspectComponent`, `InspectCollection`, `Benchmark`). | **Yes.** The prototype calls these directly. Command-line parsing moves to Python's `argparse`, and nothing from `JMain.cc` needs to be copied. |

The bundled pybind11 (2.10.3) is old. The container provides pybind11 3.0.4, which the prototype
uses.

### 1.2 How EICrecon registers factories today

* `JOmniFactoryGeneratorT<T>(tag, inputs, outputs, cfg, app)` keeps one or more *typed* wirings
  `{tag, inputs, outputs, ConfigType}`. `GenerateFactories` creates one `T` per wiring and calls
  `PreInit(tag, inputs, outputs)`.
* The **parameter prefix** is `<plugin>:<tag>`, for example `B0ECAL:B0ECalRawHits`. It is used for
  `-P<prefix>:<param>` overrides, the `:InputTags`/`:OutputTags` overrides, and the logger name.
  Algorithms are also constructed with it as their name. For example, `CalorimeterHitDigi` seeds
  its per-event RNG with `getUniqueID(header, name())`. **The plugin name therefore affects
  physics output, not just configuration.**
* Config structs are C++ aggregates set with designated initializers. Only *some* fields are
  exposed as JANA parameters through `ParameterRef`. In `CalorimeterHitDigi`, for example,
  `threshold` is not exposed. In `CalorimeterIslandCluster`, `peakNeighbourhoodMatrix`,
  `minClusterCenterEdep`, `dimScaledLocalDistXY` and `transverseEnergyProfile*` are not exposed.
  **So a string-map config bridge (the `AddWiring(map<string,string>)` overload, or JANA TOML
  wiring) cannot reproduce today's wiring.** The bridge has to reach the struct fields themselves.
* The default plugin list is a `std::vector<std::string>` in `eicrecon.cc`. Plugin order matters
  (a comment in `eicrecon_cli.cc` says so). One entry, `RICH-IRT`, is included under a
  compile-time `#if`.
* The global `japp` pointer is still used directly by `JEventProcessorPODIO`.

### 1.3 Prior art

* **Gaudi / Juggler (`gaudirun.py`)**: At build time, `genconf` generates Python `Configurable`
  classes from each component's declared `Property`s. Options files instantiate them by keyword:
  `ApplicationMgr(TopAlg=[...], ExtSvc=[...])`. Configurables know property types and names, so
  typos fail at configure time. Data handles are plain strings. EIC's earlier Juggler stack used
  this model.
* **Key4hep `k4run`**: A thin wrapper over the Gaudi configurables. A steering file sets
  `IOSvc.Input`, `Alg(InputCollections=[...], OutputCollections=[...])` and `ApplicationMgr(...)`.
  `k4run` also adds a `--<Alg>.<Property>=` command-line option for every property of every
  component in the steering file. That is the analogue of our `-P`.
* **Acts Python bindings (`acts.examples`)**: pybind11 modules. Each algorithm's `Config` struct
  is bound with a member-listing macro (`ACTS_PYTHON_STRUCT(c, inputX, outputY, ...)`) and patched
  to accept keyword arguments. `Sequencer.addAlgorithm()` checks data-handle names and types
  against earlier writers at add time and suggests close matches for typos. Reusable chains are
  Python functions (`acts.examples.reconstruction.addSeeding`, `addCKFTracks`), which is a direct
  precedent for "plugins become importable Python modules".

What the three share: Python builds a *description*, the framework validates it, then C++
executes it. Python is not involved per event.

## 2. Design

### 2.1 How Python builds the graph: a generic factory registry with reflected configs

Three options were considered:

| Option | Per-factory C++ work | Covers all config fields | Configure-time type checks |
|---|---|---|---|
| A. Hand-written pybind11 binding per factory and config struct (the Acts approach) | High: one binding per config struct, kept in sync by hand | yes | yes |
| B. Registry keyed by type name, configs as `map<string,string>` (the JANA TOML approach) | One line per factory | **no** (only `ParameterRef` fields) | no (strings) |
| C. **Registry keyed by name, configs reflected with Boost.PFR** | **One line per factory** | **yes** | **yes** |

**Recommendation: C.** Config structs are plain aggregates. Boost.PFR (≥ 1.84, C++20) provides
`for_each_field_with_name`, which gives every field's name and type with no macros. The container
has Boost 1.88. One header,
[`src/python/bindings/FactoryType.h`](../../src/python/bindings/FactoryType.h), provides:

* `FactoryType<T>`: a type-erased handle exposed to Python. It offers
  * `config_defaults()` (struct → dict),
  * `signature()` (input and output slot types and variadic flags, read from a throwaway `T`
    whose `PodioInput`/`PodioOutput` members register themselves),
  * `validate_config(dict)`,
  * `make_generator(...)`.
* `config_from_dict<Config>(dict)`: starts from `Config{}`, the same value-initialization that
  designated initializers give. It converts each given key with pybind11's type casters, and
  raises on unknown keys or values it cannot convert.
* `PyOmniFactoryGeneratorT<T>`: a `JOmniFactoryGeneratorT<T>` that restores an explicit plugin
  name in `GenerateFactories`. `JComponentManager::Add` overwrites the plugin name with the
  plugin currently loading, which is `""` for generators added from Python.

Exposing a factory to Python is then one line in a per-domain extension module
([`bindings/calorimetry.cc`](../../src/python/bindings/calorimetry.cc)):

```cpp
bind_factory<eicrecon::CalorimeterHitDigi_factory>(m, "CalorimeterHitDigi");
```

and the Python wiring reads like the C++:

```python
app.add(CalorimeterHitDigi, "B0ECalRawHits",
        ["EventHeader", "B0ECalHits"],
        ["B0ECalRawHits", "B0ECalRawHitLinks", "B0ECalRawHitAssociations"],
        eRes=[0.0326 * sqrt(u.GeV), 0.0, 0.0], threshold=5.0 * u.MeV, readout="B0ECalHits", ...)
```

Limits of PFR reflection (they all fail at compile time, never silently):

* config structs must be aggregates (no user constructors or base classes);
* every field type needs a pybind11 caster. The built-ins cover numbers, strings, `std::vector`
  and `std::variant`. Enums or nested structs would need a caster or a nested `config_to_dict`.

If C++26 static reflection becomes available, it can replace PFR with no change to the Python side.

### 2.2 How plugin and algorithm libraries are loaded

There are two kinds of C++ code, loaded in two different ways:

* **Algorithm and factory code** (`libalgorithms_calorimetry.so`, plus the header-only
  `src/factories`) is linked into a per-domain Python extension module
  (`eicrecon/_calorimetry*.so`). Importing `eicrecon.factories.calorimetry` loads exactly that
  library. A graph that only uses calorimetry never loads Acts, IRT or ONNX.
* **Services, sources and writers** (`log`, `dd4hep`, `evaluator`, `algorithms_init`, `podio`,
  ...) stay JANA plugins, added with `app.add_plugin(name)` and attached by
  `JApplication::Initialize()` as today. Unported detector plugins are loaded the same way, which
  is what allows side-by-side migration.

The extension modules are built with **default symbol visibility** (see 3.3), and are imported
with `RTLD_GLOBAL`, which `eicrecon/__init__.py` sets around
the import. This way JANA plugins opened later resolve JANA, podio and EDM symbols against the
same copies. ROOT and Gaudi do the same.

Ordering: Python generators are added to the `JApplication` **before** `Initialize()`, so they
precede all plugin generators. Factory lookup is by name, so generator order does not affect
results. Duplicate producers of one collection name are caught in Python (see 2.4). C++ plugin
order is preserved as given (`app.plugins` is an ordered list, passed as `-Pplugins`).

### 2.3 Config parameters and `-P` overrides

The precedence is unchanged:

1. The config given in Python becomes the generator's *default* config, exactly as the
   designated initializers in C++ do now.
2. In `Init()`, each `ParameterRef` registers `<plugin>:<tag>:<param>` with the
   `JParameterManager`, and any value from `-P` overrides the default. `:InputTags` and
   `:OutputTags` still work.

Because `PyOmniFactoryGeneratorT` restores the plugin name, `-PB0ECAL:B0ECalIslandProtoClusters:splitCluster=1`
means the same thing with either frontend. The Python CLI parses `-Pkey=value` with `argparse`
and passes it to `jana::CreateJApplication` unchanged.

Python graphs can also change config directly before `run()`, by editing `node.config` or by
passing different values to a chain function. Unlike `-P`, this reaches fields that are not
exposed as parameters. Units come from the C++ headers (`eicrecon.units.dd4hep`,
`eicrecon.units.edm4eic`), so `5.0 * u.MeV` is the same double in both languages. Ported wiring
keeps the `dd4hep` units the C++ wiring used (see "Unit System Policy" in `AGENTS.md`).

### 2.4 Configure-time checks

The graph stays plain Python data until `run()`, so these checks run before any plugin is loaded:

* **Unknown config field** → `GraphError` with a close-match suggestion. For example, `threshld` gives
  "did you mean 'threshold'?".
* **Wrong value type** (for example a string for a `double`) → `GraphError` naming the C++ type.
* **Wrong number of input or output names**, using the same variadic rules as `PreInit`.
* **Duplicate output collection** (two producers of one name).
* **Edge type mismatch**: a consumer's input type is not the producer's output type. Types come
  from the factories' `PodioInput<T>` and `PodioOutput<T>` declarations.
* **Dangling inputs**: `check(source_collections=...)` reports inputs that no Python node
  produces, with suggestions. Inputs from the event source or from unported C++ plugins cannot be
  checked until those are known. The prototype reports them as "external"; see the open problems.

`eicrecon-py --check` and `--dump-graph` (Graphviz) run these checks without starting JANA.

### 2.5 Shipping the default graph

```
<prefix>/python/eicrecon/
  __init__.py            Application, GraphError, units
  application.py         graph model, checks, deferred sections, run(), capture_wiring()
  cli.py                 eicrecon-py command line
  default.py             DEFAULT_PLUGINS (mirrors eicrecon.cc) + configure(app, use_cpp=())
  units.py               dd4hep / edm4eic unit constants from C++
  wiring.py              wiring capture JSON and comparison (python3 -m eicrecon.wiring a b)
  plugins/<PLUGIN>.py    Python wiring of each plugin: configure(app)
  plugins/__init__.py    bindings(module), python_module(plugin)
  chains/calorimeter.py  reusable digi→reco→cluster→shape chain
  factories/calorimetry.py   -> _calorimetry*.so, calorimetry factory types for custom graphs
  _wiring_<PLUGIN>*.so   factory types of each plugin (bindings/plugins/<PLUGIN>.cc)
  _core*.so              JApplication handle (CreateJApplication / Execute), capture, units
```

Users customize at three levels:

* `-P` overrides, unchanged.
* A graph file: `eicrecon-py --graph mygraph.py`, where `configure(app)` calls
  `eicrecon.default.configure(app)` and then adds, replaces or removes nodes.
* Copying a detector module and editing it.

Detector modules are functions, not side effects, so the same chain can be instantiated twice
under different names. That supports alternative clusterings and systematic variations.

### 2.6 Reproducibility (identical results for any thread count)

The frontend runs nothing per event. Event processing is still JANA's topology running the same
C++ factories, so the existing guarantee carries over, provided that:

* **Prefixes are identical.** The algorithm name (`<plugin>:<tag>`) seeds per-event RNGs, which
  is why `PyOmniFactoryGeneratorT` exists. A Python port must keep the C++ plugin name. The
  `with app.plugin("B0ECAL"):` block enforces that.
* **Config values are bit-identical.** Units are taken from C++, Python floats are IEEE doubles,
  and the PFR conversion writes the double into the field directly, with no string round trip.
* **Graph construction is deterministic.** Nodes and plugins are ordered lists, so nothing
  depends on `dict` or `set` iteration order or on `id()`.
* **No Python on the event path.** `execute()` releases the GIL, and no Python callbacks run
  during processing.

CI should check the reproducibility contract directly: run the Python frontend with
`nthreads=1` and `nthreads=N`, and the C++ and Python frontends, then compare outputs
collection by collection (`src/python/examples/compare_collections.py`).

### 2.7 Packaging

* The CMake option `EICRECON_PYTHON` (default `ON`) adds `src/python`. It needs Python ≥ 3.9
  (development headers), pybind11 (config package), and Boost ≥ 1.84 (headers only). If one is
  missing, CMake warns and skips the frontend instead of failing.
* Sanitizer builds: `python3` is not instrumented, so running the extension modules needs the
  sanitizer runtime preloaded (`LD_PRELOAD=$(clang++ -print-file-name=libclang_rt.asan-x86_64.so)`,
  or `libasan.so:libubsan.so` for GCC) and `detect_leaks=0` for the interpreter's own leaks. The
  ctest `t_python_frontend` does this automatically; it is skipped in TSAN builds.
* The package installs to `<prefix>/python/eicrecon`. The `EICRECON_PYTHON_INSTALL_DIR` cache
  variable sets that path. The launcher `eicrecon-py` goes to `<prefix>/bin`.
* The extension modules get `INSTALL_RPATH=<prefix>/lib`, so they find the algorithm libraries
  without `LD_LIBRARY_PATH`.
* `eicrecon-this.sh` prepends `<prefix>/python` to `PYTHONPATH` when it exists, next to the
  existing `PATH`, `LD_LIBRARY_PATH`, `JANA_PLUGIN_PATH` and `ROOT_INCLUDE_PATH` handling.

## 3. Prototype

### 3.1 What was built

* `src/python/bindings/{FactoryType.h,core.cc,calorimetry.cc}`: the generic factory binding, the
  `JApplication` handle, and the six calorimetry factory types used by B0ECAL.
* `src/python/eicrecon/...`: the graph model, checks, CLI, default graph, a reusable calorimeter
  chain, and the B0ECAL port.
* `src/python/examples/b0ecal_only.py`: a minimal graph with five service plugins plus the
  B0ECAL chain.
* `src/python/examples/compare_collections.py`: a branch-by-branch exact comparison of podio
  collections.

### 3.2 Test

Sample: 100 events, a 20 GeV photon gun into the B0 acceptance (θ = 10–35 mrad, uniform φ), in
the `epic_craterlake` geometry of the `eic_xl:nightly` image. 92 of the 100 events have
`B0ECalHits`. Commands, with `ddsim` as in `AGENTS.md`:

```bash
ddsim --compactFile $DETECTOR_PATH/$DETECTOR_CONFIG.xml --numberOfEvents 100 --enableGun \
  --gun.thetaMin '0.010*rad' --gun.thetaMax '0.035*rad' --gun.distribution uniform \
  --gun.phiMin '0*deg' --gun.phiMax '360*deg' --gun.energy '20*GeV' --gun.particle gamma \
  --random.seed 42 --outputFile b0gun.edm4hep.root
# reference: unchanged C++ eicrecon, full default plugin list
eicrecon -Pnthreads=1 -Ppodio:output_collections=EventHeader,B0ECal... b0gun.edm4hep.root
# Python, minimal graph: 6 service plugins + B0ECAL chain from Python
eicrecon-py --graph src/python/examples/b0ecal_only.py -Pnthreads=4 b0gun.edm4hep.root
# Python, default graph: all C++ plugins except B0ECAL, which is wired from Python
eicrecon-py -Pnthreads=4 -Ppodio:output_collections=EventHeader,B0ECal... b0gun.edm4hep.root
# compare all 13 collections (EventHeader + 12 B0ECal*), event by event, every branch exactly
python3 src/python/examples/compare_collections.py ref.edm4eic.root test.edm4eic.root ...
```

| Comparison (vs C++ `eicrecon`, 1 thread) | Collections bit-identical |
|---|---|
| C++ `eicrecon`, 4 threads | 13/13 |
| Python minimal graph, 1 thread | 13/13 |
| Python minimal graph, 4 threads | 13/13 |
| Python default graph (C++ plugins + Python B0ECAL), 4 threads | 13/13 |
| *Negative control:* Python minimal graph with plugin name `B0ECAL_RENAMED` | 1/4 (`EventHeader` only; `RawHits` amplitudes differ) |

That covers 3776 raw hits, 2010 reconstructed hits, 130 island clusters and 90 truth clusters,
including all relation branches and associations. The negative control shows that the comparison
can detect a difference: a different plugin name changes the algorithm name, and so the RNG seeds
in `CalorimeterHitDigi`.

`eicrecon-py -c` lists the factory parameters under the same keys as `eicrecon`, for example
`B0ECAL:B0ECalIslandProtoClusters:dimScaledLocalDistXY`.

Timing, wall clock for the whole job (8-core host, mostly geometry and service initialization):

| Job | Wall time | Max RSS |
|---|---|---|
| C++ `eicrecon`, full default plugins, 1 / 4 threads | 38.1 / 38.3 s | 2.47 / 2.51 GB |
| Python minimal graph, 1 / 4 threads | 29.9 / 28.3 s | 2.44 / 2.48 GB |
| Python default graph, 4 threads | 39.5 s | 2.51 GB |

Importing `eicrecon` plus `eicrecon.factories.calorimetry` takes about 0.6 s. The minimal graph
saves about 9 s by loading only 6 plugins. Most of the remaining time is DD4hep and Acts geometry
loading (see below for why `acts` is still needed). Python adds no measurable overhead per event.

Configure-time checks, exercised in the smoke test, each raise `GraphError` before JANA starts:

```
T:X: CalorimeterHitDigi has no config field 'threshld' (did you mean 'threshold'?)
T:X: cannot convert value for config field 'capADC' to C++ type unsigned int
T:X: expected 2 input collection names, got 1
T:Y: output 'ARecHits' is already produced by T:ARecHits
T:Isl: input 'ARawHits' has type edm4eic::CalorimeterHit, but is produced as edm4hep::RawCalorimeterHit
inputs not produced by the graph or the source: 'B0ECalHits' (did you mean 'B0ECalHitz'?)
```

### 3.3 Problems found while prototyping

1. **Symbol visibility duplicates singletons.** `pybind11_add_module` compiles with
   `-fvisibility=hidden`. The `algorithms` services are header-defined function-local static singletons
   (`algorithms::GeoSvc::instance()`), and `CalorimeterHitDigi` binds `m_geo` in a default member
   initializer, which is inlined into the extension module. The module therefore got its **own,
   never-initialized** `GeoSvc`, and `init()` aborted on a `gsl::not_null`. Fix: build the
   factory modules with default visibility, as the JANA plugins are (`src/python/CMakeLists.txt`).
   Any future binding module must keep this.
2. **Hidden plugin dependencies.** The minimal graph first failed with
   `Service not found: 'ACTSGeo_service'`. `algorithms_init` unconditionally acquires the Acts
   geometry, so a calorimeter-only job must load `acts`, which also costs time. See open problem 2.
3. **Events are written in completion order** with `nthreads > 1`. Output files must be compared
   event by event (by `EventHeader.eventNumber`), not entry by entry. A first naive comparison
   showed spurious differences even between two C++ runs.

### 3.4 What the prototype does not show

* It covers one detector and six factory types, all with simple aggregate configs. Tracking,
  PID and global factories (Acts types, ONNX, IRT, variadic inputs) are not yet bound.
* `-P` overrides go through the same parameter keys, but no run compared a `-P` override between
  the two frontends.
* The JANA modes `--benchmark`, inspector and `--list-available-plugins` are not wired.
* Inputs from the event source and C++ plugins are reported, not validated (open problem 1).
* The comparison covered 100 single-particle events, not DIS events or the `eicrecon-gun`/`-dis`
  CI samples.

## 4. Open problems

1. **Inputs from the event source and from C++ plugins are not checked at configure time.**
   Python only knows what Python nodes produce. Options:
   * open the first input file in Python (podio's Python reader) to list its collections;
   * have every plugin export a machine-readable list of its outputs (`--list-collections` output
     in JSON) at build time.

   Once all plugins are ported, only the source remains, and the first option is enough.
2. **Plugin dependency ordering.** C++ plugins depend on each other's services and on order
   (`eicrecon_cli.cc`: "the sequence of adding the default plugins matters"). Python keeps the
   list order, but nothing declares dependencies. Python modules can declare
   `REQUIRES = ["dd4hep", "algorithms_init"]` and `app.add_plugin` can resolve them topologically.
   Then the minimal graph in `b0ecal_only.py` would no longer list services by hand. The
   prototype hit this: `algorithms_init` needs `acts` even for calorimeter-only jobs. Making that
   dependency lazy in `AlgorithmsInit_service` would also cut start-up time.
3. **Optional inputs** (`PodioInput<T, true>`) are not visible in the signature. `InputBase` does
   not record `IsOptional`. Adding a bool there, upstream in `JOmniFactory.h`, would let the
   dangling-input check skip them.
4. **Compile-time conditionals** in the wiring (`#if EDM4EIC_VERSION_MAJOR >= 8`, IRT2 versions,
   ONNX availability) become run-time checks in Python. `_core` should export the build's version
   macros, for example `eicrecon._core.versions["edm4eic"]`.
5. **Non-aggregate configs and exotic field types** need pybind11 casters. Survey all
   `*Config.h` structs before porting (see migration phase 2).
6. **Performance.** Building the graph takes milliseconds and nothing runs per event, so runtime
   cost is nil in principle. Two costs to measure:
   * import time of the extension modules (each pulls in its algorithm libraries, about what
     plugin loading costs today);
   * the throwaway factory constructed per type in `signature()`. Signatures are cached per type.
7. **Inspector, benchmark and `--list-available-plugins` modes.** `execute()` already maps
   `configs`, `list-factories` and `list-collections` to JANA's flags. Benchmark mode and the
   interactive inspector are one line each.
8. **`japp`.** `_core` sets the global `japp`, because `JEventProcessorPODIO` still reads it.
   Only one `Application` can run per process.
9. **JANA2 upstream.** EICrecon's `JOmniFactory` is a fork of JANA2's. Converging on JANA2's
   `JOmniFactory`/`JWiringService` would let the Python layer emit wirings into JANA's own service
   instead of creating generators. It would still need typed configs (section 2.1).

## 5. Migration plan

**Phase 0 (this prototype).** Build behind `EICRECON_PYTHON=OFF`, plus one ported detector
(B0ECAL) and the comparison tool. No user-visible change.

**Phase 1: run both frontends side by side.** *(done)*
* `EICRECON_PYTHON` is on by default; `eicrecon-py` ships next to `eicrecon`.
* `eicrecon-py` accepts every `eicrecon` option with the same meaning and precedence: `-h`, `-v`,
  `-j`, `-c`, `-l [file]`, `-d [file]`, `-b`, `-L`, `--list-default-plugins`,
  `--list-available-plugins`, `-Pkey=value` (first occurrence wins, as in `eicrecon`),
  `-Pplugins=`, and `-Pplugins_to_ignore=` including the `default` keyword. It applies the same
  run-mode defaults: `jana:timeout` and `jana:warmup_timeout` of 180 s unless set, and
  `jana:parameter_strictness=2` when running, plus `$EICrecon_MY/plugins` on the plugin path.
  Run, benchmark and config-dump modes go through `jana::Execute`; the `-c` and `-L` tables are
  printed by Python from data that `_core` returns, so none of `eicrecon_cli.cc` is reused.
  Deliberate differences: a malformed `-P` or an unknown flag exits with status 2 instead of
  printing the parameter table or usage with status 0; the `-c` and `-L` tables use a different
  text layout with the same content.
* `eicrecon-py` loads every unported detector as a C++ plugin. `--use-cpp B0ECAL` switches a
  ported detector back for bisection.
* CI job `eicrecon-py` runs the Python frontend on the inputs of `eicrecon-gun` and
  `eicrecon-dis`: the e⁻ and π⁻ gun samples (1 and 4 threads) and the 18×275, Q² > 100 DIS
  sample (4 threads), with the same ASAN build and options. `compare-cpp-python-frontends`
  compares its outputs with the C++ ones (single-threaded): a capybara report, and a check
  that every written collection is bitwise identical
  (`src/python/examples/compare_collections.py --all`, events matched by run and event number).
* ctest `t_python_frontend` checks the graph layer and the command line without event
  processing.

Phase 1 validation (local, `eic_xl:nightly`, full default graph, all collections):

| Sample | Threads | Collections bitwise identical |
|---|---|---|
| e⁻ gun, 1–20 GeV, 3°–177°, 100 events | 1 | 439/439 |
| π⁻ gun, 1–20 GeV, 3°–177°, 100 events | 4 | 439/439 |
| B0 photon gun with `-PB0ECAL:...:splitCluster=1 -PB0ECAL:...:samplingFraction=0.9` | 4 | 439/439 |
| DIS 18×275, Q² > 100, 20 events | 4 | 439/439 |

The overrides change the B0ECal output relative to the run without them (11 of 12 B0ECal
collections differ), so both frontends apply `-P` overrides of Python-wired factories.
`-c` lists the same 2731 parameter names, `-L` the same 729 factories, and a file written with `-d` and
read back with `-l` restores the values. One DIS event has `InclusiveKinematicsElectron.W = NaN`
in both frontends. The first comparison flagged it because NaN ≠ NaN, so the comparison is now
bitwise. The NaN itself is being investigated separately.

**Phase 2: Python wiring for every plugin.** *(done)*

Every plugin with `InitPlugin` wiring (24 detectors, 6 global) has a module
`eicrecon.plugins.<PLUGIN>`, and the default graph uses them: the C++ plugins that are loaded
provide only services, sources and writers. The C++ `InitPlugin` functions still exist and
`--use-cpp <PLUGIN>|all` loads them instead.

* *Binding modules per plugin.* `tools/generate_bindings.py` writes
  `bindings/plugins/<PLUGIN>.cc` with the plugin's own `#include`s and one `bind_factory` per
  factory type it wires (253 bindings, 30 modules). CMake builds each with the plugin's include
  directories, definitions, link directories and link libraries, read from the plugin target.
* *Wiring capture.* `JOmniFactoryGeneratorT::GenerateFactories` reports every wiring it turns
  into factories (plugin, tag, collections, config struct) to an optional observer
  (`extensions/jana/JOmniFactoryObserver.h`; one pointer check when unused). `eicrecon-py
  --capture-wiring file.json` installs one, initializes the job and writes the wirings with
  their config structs converted by Boost.PFR, from C++ plugins and Python alike.
* *Conversion.* `tools/generate_wiring.py` turns a capture of the C++ plugins into Python
  modules. Fields equal to the struct defaults are left out. A value is written as the C++
  initializer expression translated to Python (`5.0 * u.MeV`, local constants included) when
  that evaluates to exactly the captured value, and as an exact literal otherwise. 24 modules
  are generated this way (308 wirings).
* *Geometry-dependent wiring* is written by hand as *deferred sections*:
  `app.deferred(fn)` runs `fn(graph)` while JANA generates factories, with geometry queries
  (`graph.constant_int`, `constant_double`, `readout_segmentation_type`) and parameters
  (`graph.parameter`). FEMC (homogeneous or ScFi digitization), FHCAL (insert clustering),
  MPGD (pixel or strip digitization, `MPGD:SiFactoryPattern`), DRICH and RICH-IRT (tracking
  planes) use them. Values that only C++ can compute come from small provider functions in
  the binding modules (`T.track_propagation_configs`, `T.rich_configs`).
* *Config values Python cannot write.* `std::function` fields are `CppFunction` objects,
  created by constructors in the binding modules (`T.GeometrySplit_TrackerHit(...)`,
  `T.ValueSplit_GeneratorStatus_PDG(...)`, `T.TrackPointSystemSplit(...)`) or by providers.
  Enums are integers, nested structs dicts, `std::variant` alternatives that are structs
  tagged dicts (`{"type": "DiscSurfaceConfig", ...}`).
* *C++ changes needed* (behavior unchanged): factory types must be nameable outside their
  plugin, so the lambdas used as `FilterMatching_factory` template arguments became named
  accessors (`factories/meta/ObjectIDAccessors.h`), `reco_particle_pt` moved to a header, and
  the `particle_flow` projection split became the functor `TrackPointSystemSplit`. The RICH-IRT
  configuration code moved to `detectors/RICH-IRT/RichIrtConfig.h`, shared by the plugin and
  its Python wiring.
* *CI.* Job `compare-cpp-python-wiring` captures the wirings of the C++ plugins and of the
  Python graph for seven detector configurations that cover the geometry-dependent branches,
  and requires them to be identical. Job `compare-cpp-python-frontends` (phase 1) now compares
  `eicrecon` with the all-Python graph.

Phase 2 validation (local, `eic_xl:nightly`):

| Wiring capture, C++ plugins vs Python graph | Wirings | Differences | Branch exercised |
|---|---|---|---|
| `epic_craterlake` | 359 | 0 | default |
| `epic_craterlake_18x275` | 359 | 0 | |
| `epic_craterlake_tracking_2DStrip` | 356 | 0 | MPGD strip digitization |
| `epic_craterlake_tracking_only` | 356 | 0 | FHCAL insert HEXPLIT clustering (no insert readout) |
| `epic_femcal_scfi` | 356 | 0 | FEMC ScFi digitization |
| `epic_forward_insert_only` | 355 | 0 | |
| `epic_drich_only` / `epic_pfrich_only` | 358 / 358 | 0 / 0 | RICH-IRT with one RICH; DRICH absent |

With the default graph wired entirely from Python (only service, source and writer plugins
loaded as C++), the outputs of `eicrecon` and `eicrecon-py` are bitwise identical, as are the
Python outputs at 1 and 4 threads:

| Sample | Threads | Collections identical |
|---|---|---|
| e⁻ gun, 100 events | 1 | 439/439 |
| π⁻ gun, 100 events | 4 | 439/439 |
| DIS 18×275, Q² > 100, 20 events | 4 and 1 | 439/439, 439/439 |
| DIS, `eicrecon-py` 1 thread vs 4 threads | | 439/439 |

Wall time and memory are the same within 2% (about 96 s and 2.8–3.2 GB per job, dominated by
initialization). Not covered by a geometry in the image: FEMC with
`EcalEndcapP_Homogeneous_ScFi > 2`, where neither frontend adds a digitization.

**Phase 3: switch over.**
* `eicrecon` becomes the Python launcher. Delete `eicrecon.cc` and `eicrecon_cli.cc`, which also
  removes the code copied from `JMain.cc`.
* Remove `EICRECON_DEFAULT_PLUGINS`.
* Detector `InitPlugin` files are gone. Remaining plugins provide only services, sources and
  writers.
* Keep the `-P` and plugin-name compatibility indefinitely: users' scripts and the benchmarks
  rely on `-P<plugin>:<tag>:<param>`.

**Breaking-change notes.** Phases 0–2 are additive. Phase 3 changes the executable (it gains a
Python dependency, and `--list-available-plugins` output may change), so it needs a
`BREAKING CHANGE:` footer per `AGENTS.md`.
