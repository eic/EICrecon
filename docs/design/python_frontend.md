# Python frontend for EICrecon (design exploration)

Status: **exploration / prototype**. Nothing in the existing C++ frontend has been removed. The
prototype is built only with `-DEICRECON_PYTHON=ON`.

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
  application.py         graph model, checks, run()
  cli.py                 eicrecon-py command line
  default.py             DEFAULT_PLUGINS (mirrors eicrecon.cc) + configure(app, use_cpp=())
  units.py               dd4hep / edm4eic unit constants from C++
  factories/calorimetry.py   -> _calorimetry*.so (one module per algorithm domain)
  detectors/__init__.py      PORTED = {"B0ECAL": "eicrecon.detectors.b0ecal", ...}
  detectors/calorimeter_chain.py   reusable digi→reco→cluster→shape chain
  detectors/b0ecal.py        port of src/detectors/B0ECAL/B0ECAL.cc
  _core*.so                  JApplication handle (CreateJApplication / Execute)
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

* The CMake option `EICRECON_PYTHON` (default `OFF` while this is a prototype) adds `src/python`.
  It needs Python ≥ 3.9 (development headers), pybind11 (config package), and Boost ≥ 1.84
  (headers only).
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

**Phase 1: run both frontends side by side.**
* Turn `EICRECON_PYTHON` on in CI and ship `eicrecon-py`.
* Add CI jobs that run `eicrecon` and `eicrecon-py` on the `eicrecon-gun` and `eicrecon-dis`
  samples, and require the outputs to be identical collection by collection. Also add an
  `nthreads` matrix.
* `eicrecon-py` loads every unported detector as a C++ plugin. `--use-cpp B0ECAL` switches a
  ported detector back for bisection.

**Phase 2: generate Python wiring from the C++ plugins.**
* Write a one-off converter (libclang or clang `-ast-dump=json`) for the `InitPlugin` bodies. It
  extracts each `JOmniFactoryGeneratorT<T>(tag, {inputs}, {outputs}, {designated init})`, maps
  `dd4hep::X` to `u.X`, and emits a Python module per plugin plus the `bind_factory` lines per
  domain.
* Hand-port the irregular parts the converter cannot handle: loops, shared `decltype` constants,
  `#if` blocks, lambdas.
* Port domain by domain: calorimetry → tracking → PID → global. Each port must pass the
  identical-output CI gate before its C++ `InitPlugin` is deleted.
* Keep each C++ plugin library for its services or event sources, if it has any.

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
