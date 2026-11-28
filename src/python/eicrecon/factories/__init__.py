# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 Wouter Deconinck
"""Factory types, one submodule per factory domain (src/factories/<domain>).

``eicrecon.factories.<domain>`` holds the factory types of the extension module
``eicrecon._<domain>`` (built from src/python/bindings/<domain>/), and the C++ helpers for
config values that Python cannot write. Importing a submodule loads only the C++ libraries of
that domain, e.g. ``eicrecon.factories.calorimetry`` loads libalgorithms_calorimetry.so.
"""

import importlib
import os
import sys

DOMAINS = ["calorimetry", "digi", "fardetectors", "meta", "particle_flow", "pid", "pid_lut",
           "reco", "tracking"]


def _load(domain):
    """The public names of eicrecon._<domain>.

    The module is loaded with RTLD_GLOBAL, so that singletons defined in headers (e.g.
    algorithms::GeoSvc::instance()) are shared with the JANA plugins and the other modules."""
    flags = sys.getdlopenflags()
    sys.setdlopenflags(flags | os.RTLD_GLOBAL)
    try:
        module = importlib.import_module(f"eicrecon._{domain}")
    finally:
        sys.setdlopenflags(flags)
    return {k: getattr(module, k) for k in dir(module) if not k.startswith("_")}


def import_all():
    """Import every domain, so that all factory types are known (for wiring capture)"""
    return [importlib.import_module(f"{__name__}.{d}") for d in DOMAINS]
