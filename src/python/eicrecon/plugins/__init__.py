# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 Wouter Deconinck
"""Python wiring of the EICrecon plugins: one module per plugin, each with ``configure(app)``.

Module names are the plugin names with non-identifier characters replaced by '_'
(``RICH-IRT`` -> ``RICH_IRT``). The factory types come from eicrecon.factories.<domain>. Most
modules were converted from the former C++ InitPlugin functions; those whose wiring depends on
the geometry were written by hand and use deferred sections (Application.deferred).
"""

import pkgutil
import re


def module_name(plugin):
    return re.sub(r"\W", "_", plugin)


def _ported():
    found = {}
    for info in pkgutil.iter_modules(__path__):
        found[info.name] = f"{__name__}.{info.name}"
    return found


_MODULES = _ported()


def python_module(plugin):
    """Module path of the Python wiring of `plugin`, or None if it has none"""
    return _MODULES.get(module_name(plugin))
