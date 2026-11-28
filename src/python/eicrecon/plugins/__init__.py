# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 Wouter Deconinck
"""Python wiring of the EICrecon plugins: one module per plugin, each with ``configure(app)``.

Module names are the plugin names with non-identifier characters replaced by '_'
(``RICH-IRT`` -> ``RICH_IRT``). Most modules were converted from the C++ InitPlugin functions
by src/python/tools/generate_wiring.py; those whose wiring depends on the geometry were
written by hand.
"""

import importlib
import os
import pkgutil
import re
import sys


def module_name(plugin):
    return re.sub(r"\W", "_", plugin)


def bindings(module):
    """The binding module of a plugin (eicrecon._wiring_<module>), with its factory types"""
    flags = sys.getdlopenflags()
    sys.setdlopenflags(flags | os.RTLD_GLOBAL)
    try:
        return importlib.import_module(f"eicrecon._wiring_{module}")
    finally:
        sys.setdlopenflags(flags)


def _ported():
    found = {}
    for info in pkgutil.iter_modules(__path__):
        found[info.name] = f"{__name__}.{info.name}"
    return found


_MODULES = _ported()


def python_module(plugin):
    """Module path of the Python wiring of `plugin`, or None if it has none"""
    return _MODULES.get(module_name(plugin))




def import_all_bindings():
    """Import every binding module, so that all factory types are known (for wiring capture)"""
    import eicrecon
    pkg_dir = os.path.dirname(eicrecon.__file__)
    modules = []
    for f in sorted(os.listdir(pkg_dir)):
        m = re.match(r"_wiring_(\w+?)\.", f)
        if m:
            modules.append(bindings(m.group(1)))
    return modules
