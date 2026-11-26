# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 Wouter Deconinck
"""The default EICrecon graph: service, source and writer plugins (C++), and the factory wiring
of the detector and global plugins (Python, eicrecon.plugins).

Copy this module (or call ``configure`` and then modify ``app``) to customize the graph.
"""

import importlib

from .plugins import python_module

# Default plugins, in order. Names with a module in eicrecon.plugins are wired from Python;
# the others are JANA plugins (services, sources, writers). The order matters for the
# JANA plugins.
DEFAULT_PLUGINS = [
    "log", "dd4hep", "evaluator", "acts", "algorithms_init", "pid_lut", "richgeo", "rootfile",
    "beam", "reco", "tracking", "particle_flow", "pid", "global_pid_lut",
    "EEMC", "BEMC", "FEMC", "EHCAL", "BHCAL", "FHCAL", "B0ECAL", "ZDC",
    "BTRK", "BVTX", "DIRC", "DRICH", "RICH-IRT", "ECTRK", "MPGD", "B0TRK", "RPOTS",
    "FOFFMTRK", "BTOF", "ECTOF", "LOWQ2", "LUMISPECCAL", "LUMISPECTRK", "LUMIDIRECTPCAL",
    "podio", "janatop",
]


def configure(app):
    """Add the default plugins and their Python wiring to `app`.

    Honors -Pplugins_to_ignore, including the 'default' keyword, and wires the names in
    -Pplugins that have Python wiring (other names are loaded as JANA plugins).
    """
    ignore = [p for p in app.params.get("plugins_to_ignore", "").split(",") if p]
    if "default" in ignore:
        # 'default' is a keyword of this frontend, not a plugin name: do not pass it to JANA
        ignore = [p for p in ignore if p != "default"]
        if ignore:
            app.params["plugins_to_ignore"] = ",".join(ignore)
        else:
            del app.params["plugins_to_ignore"]
        plugins = []
    else:
        plugins = [p for p in DEFAULT_PLUGINS if p not in ignore]

    # -Pplugins=...: extra plugins; those with Python wiring are wired here, not loaded by JANA
    extra = [p for p in app.params.pop("plugins", "").split(",") if p]
    jana_extra = []
    for name in extra:
        if python_module(name):
            if name not in plugins:
                plugins.append(name)
        else:
            jana_extra.append(name)
    if jana_extra:
        app.params["plugins"] = ",".join(jana_extra)

    for name in plugins:
        module = python_module(name)
        if module:
            importlib.import_module(module).configure(app)
        else:
            app.add_plugin(name)
