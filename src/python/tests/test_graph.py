# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 Wouter Deconinck
"""Tests of the eicrecon Python frontend that need no geometry or input file.

Run by ctest (t_python_frontend) against the package assembled in the build tree.
"""

import contextlib
import io
import sys

import eicrecon
from eicrecon import Application, GraphError
from eicrecon.cli import UsageError, main, parse_args
from eicrecon.factories import calorimetry as calo

failures = []


def check(name, condition, detail=""):
    print(("PASS " if condition else "FAIL ") + name + (f": {detail}" if detail else ""))
    if not condition:
        failures.append(name)


def raises(fn, exc=GraphError, contains=""):
    try:
        fn()
    except exc as e:
        return contains in str(e), str(e)
    return False, "no exception"


# --- factory introspection -----------------------------------------------------------------
sig = calo.CalorimeterHitDigi.signature()
check("signature inputs", [t for t, _ in sig["inputs"]] ==
      ["edm4hep::EventHeader", "edm4hep::SimCalorimeterHit"], sig["inputs"])
defaults = calo.CalorimeterHitDigi.config_defaults()
check("config defaults reflect all fields", {"eRes", "threshold", "readout"} <= set(defaults))
check("config default values", defaults["readoutType"] == "simple", defaults["readoutType"])

# --- units come from C++ ---------------------------------------------------------------------
u = eicrecon.units
check("dd4hep units", u.dd4hep.mm == 0.1 and u.dd4hep.GeV == 1.0)
check("edm4eic units", u.edm4eic.mm == 1.0 and u.edm4eic.MeV == 1e-3)

# --- configure-time checks -------------------------------------------------------------------
app = Application()
with app.plugin("T"):
    app.add(calo.CalorimeterHitReco, "ARecHits", ["ARawHits"], ["ARecHits"], readout="A")
    ok, msg = raises(lambda: app.add(calo.CalorimeterHitDigi, "X", ["EventHeader", "H"],
                                     ["a", "b", "c"], threshld=1.0), contains="'threshold'")
    check("unknown config field suggests a match", ok, msg)
    ok, msg = raises(lambda: app.add(calo.CalorimeterHitDigi, "X", ["EventHeader", "H"],
                                     ["a", "b", "c"], capADC="lots"), contains="unsigned int")
    check("wrong config type", ok, msg)
    ok, msg = raises(lambda: app.add(calo.CalorimeterHitDigi, "X", ["H"], ["a", "b", "c"]),
                     contains="expected 2 input")
    check("wrong input count", ok, msg)
    ok, msg = raises(lambda: app.add(calo.CalorimeterHitReco, "Y", ["ARawHits"], ["ARecHits"]),
                     contains="already produced")
    check("duplicate output", ok, msg)
    app.add(calo.CalorimeterIslandCluster, "Isl", ["ARawHits"], ["AProto"])
    app.add(calo.CalorimeterHitDigi, "ARaw", ["EventHeader", "AHits"],
            ["ARawHits", "ARawHitLinks", "ARawHitAssociations"])
ok, msg = raises(app.check, contains="produced as edm4hep::RawCalorimeterHit")
check("edge type mismatch", ok, msg)
ok, msg = raises(lambda: Application().add(calo.CalorimeterHitReco, "Z", ["a"], ["b"]),
                 contains="app.plugin")
check("factories need a plugin name", ok, msg)

# --- ported detector ------------------------------------------------------------------------
from eicrecon.detectors import b0ecal  # noqa: E402

app = Application()
b0ecal.configure(app)
check("B0ECAL external inputs", app.check() == ["EventHeader", "B0ECalHits"], app.check())
check("B0ECAL prefix", app.nodes[0].prefix == "B0ECAL:B0ECalRawHits", app.nodes[0].prefix)
ok, msg = raises(lambda: app.check(source_collections=["EventHeader", "B0ECalHitz"]),
                 contains="did you mean 'B0ECalHitz'")
check("dangling input", ok, msg)

# --- default graph and plugins_to_ignore ----------------------------------------------------
from eicrecon import default  # noqa: E402

app = Application({"plugins_to_ignore": "B0ECAL,janatop"})
default.configure(app)
check("ignored ported detector is not wired", not app.nodes)
check("ignored C++ plugin is not loaded", "janatop" not in app.plugins)
app = Application({"plugins_to_ignore": "default,foo"})
default.configure(app)
check("'default' ignores all defaults", not app.plugins and not app.nodes)
check("'default' is not passed to JANA", app.params["plugins_to_ignore"] == "foo",
      app.params["plugins_to_ignore"])
app = Application()
default.configure(app, use_cpp=["B0ECAL"])
check("--use-cpp loads the C++ plugin", "B0ECAL" in app.plugins and not app.nodes)

# --- command line ---------------------------------------------------------------------------
o = parse_args(["-Pnthreads=4", "-Pnthreads=8", "-l", "my.config", "-d", "-L", "in.root"])
check("first -P wins (as in eicrecon)", o.params == {"nthreads": "4"}, o.params)
check("-l takes a file", o.load_config_file == "my.config")
check("-d without file", o.dump_config_file == "jana.config" and o.sources == ["in.root"],
      (o.dump_config_file, o.sources))
check("flags", {"loadconfigs", "dumpconfigs", "list-factories"} <= o.flags, o.flags)
o = parse_args(["--graph", "g.py", "--use-cpp=B0ECAL", "x.root"])
check("graph options", o.graph == "g.py" and o.use_cpp == ["B0ECAL"] and o.sources == ["x.root"])
ok, msg = raises(lambda: parse_args(["-Pnoequals"]), exc=UsageError, contains="-Pkey=value")
check("malformed -P", ok, msg)
ok, msg = raises(lambda: parse_args(["--bogus"]), exc=UsageError, contains="--bogus")
check("unknown flag", ok, msg)
check("no arguments shows usage", "help" in parse_args([]).flags)

out = io.StringIO()
with contextlib.redirect_stdout(out):
    rc = main(["--check"])
check("--check on default graph", rc == 0 and "B0ECalHits" in out.getvalue(), out.getvalue())
out = io.StringIO()
with contextlib.redirect_stdout(out):
    rc = main(["-v"])
check("--version", rc == 0 and "EICrecon version" in out.getvalue(), out.getvalue())

print(f"{len(failures)} failures")
sys.exit(1 if failures else 0)
