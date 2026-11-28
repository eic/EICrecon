# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 Wouter Deconinck
"""The default EICrecon graph: the plugin list of src/utilities/eicrecon/eicrecon.cc, with
detectors that have a Python port (eicrecon.detectors.PORTED) wired from Python instead.

Copy this module (or call ``configure`` and then modify ``app``) to customize the graph.
"""

import importlib
import os

from .detectors import PORTED

# Keep in sync with EICRECON_DEFAULT_PLUGINS in src/utilities/eicrecon/eicrecon.cc.
# The order matters for the C++ plugins (see eicrecon_cli.cc).
DEFAULT_PLUGINS = [
    "log", "dd4hep", "evaluator", "acts", "algorithms_init", "pid_lut", "richgeo", "rootfile",
    "beam", "reco", "tracking", "particle_flow", "pid", "global_pid_lut",
    "EEMC", "BEMC", "FEMC", "EHCAL", "BHCAL", "FHCAL", "B0ECAL", "ZDC",
    "BTRK", "BVTX", "DIRC", "DRICH", "RICH-IRT", "ECTRK", "MPGD", "B0TRK", "RPOTS",
    "FOFFMTRK", "BTOF", "ECTOF", "LOWQ2", "LUMISPECCAL", "LUMISPECTRK", "LUMIDIRECTPCAL",
    "podio", "janatop",
]

# Built only with IRT2 >= 2.2 (compile-time condition in eicrecon.cc)
OPTIONAL_PLUGINS = {"RICH-IRT"}


def _plugin_available(name):
    for d in os.environ.get("JANA_PLUGIN_PATH", "").split(":"):
        if d and os.path.exists(os.path.join(d, name + ".so")):
            return True
    return False


def configure(app, use_cpp=()):
    """Add the default plugins and Python-ported detectors to `app`.

    `use_cpp` lists ported detectors to load from their C++ plugin instead (side by side).
    Honors -Pplugins_to_ignore (including the 'default' keyword) like eicrecon.
    """
    ignore = [p for p in app.params.get("plugins_to_ignore", "").split(",") if p]
    if "default" in ignore:
        # 'default' is a keyword of this frontend, not a plugin name: do not pass it to JANA
        rest = [p for p in ignore if p != "default"]
        if rest:
            app.params["plugins_to_ignore"] = ",".join(rest)
        else:
            del app.params["plugins_to_ignore"]
        return
    for name in DEFAULT_PLUGINS:
        if name in ignore:
            continue
        if name in OPTIONAL_PLUGINS and not _plugin_available(name):
            continue
        if name in PORTED and name not in use_cpp:
            importlib.import_module(PORTED[name]).configure(app)
        else:
            app.add_plugin(name)
