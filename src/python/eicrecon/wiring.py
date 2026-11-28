# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 Wouter Deconinck
"""Captured factory wirings (eicrecon --capture-wiring) as JSON, and their comparison.

A wiring is {plugin, tag, cpp_type, type, inputs, outputs, config}. Config values are exact:
floats are written with repr(), so they read back bit for bit.

std::function config values are recorded by the type of their target (e.g. a named functor or a
lambda). Two functions of the same target type may still hold different state, so such pairs are
reported as unverified rather than as equal.
"""

import json
import math

from . import _core


def _plain(value):
    if isinstance(value, _core.CppFunction):
        return {"cpp_function": value.type_name, "description": value.description}
    if isinstance(value, dict):
        return {k: _plain(v) for k, v in value.items()}
    if isinstance(value, (list, tuple)):
        return [_plain(v) for v in value]
    return value


def write_wiring(records, path):
    with open(path, "w") as f:
        json.dump([_plain(r) for r in records], f, indent=1, allow_nan=True)


def read_wiring(path):
    with open(path) as f:
        return json.load(f)


def _is_function(value):
    return isinstance(value, dict) and set(value) == {"cpp_function", "description"}


def _same(a, b, unverified=None, path=""):
    """Whether a and b are equal; nonempty functions of the same type are equal, but their
    paths are appended to `unverified`."""
    if isinstance(a, float) and isinstance(b, float):
        return a == b or (math.isnan(a) and math.isnan(b))
    if type(a) is not type(b):
        return False
    if _is_function(a) and _is_function(b):
        if a != b:
            return False
        if a["description"] != "empty" and unverified is not None:
            unverified.append(path)
        return True
    if isinstance(a, dict):
        return a.keys() == b.keys() and all(
            _same(a[k], b[k], unverified, f"{path}.{k}") for k in a)
    if isinstance(a, list):
        return len(a) == len(b) and all(
            _same(x, y, unverified, f"{path}[{i}]") for i, (x, y) in enumerate(zip(a, b)))
    return a == b


def compare_wiring(ref, test, unverified=None):
    """Return a list of differences between two lists of wirings (empty if identical).

    Config values that cannot be compared (functions of the same target type) are appended to
    `unverified`, if given, as '<plugin>:<tag>:<field>' paths.
    """
    key = lambda r: (r["plugin"], r["tag"])  # noqa: E731
    a, b = {key(r): r for r in ref}, {key(r): r for r in test}
    diffs = []
    for name, records in (("reference", ref), ("test", test)):
        for r in records:
            if r.get("duplicate"):
                diffs.append(f"{r['plugin']}:{r['tag']} is wired twice in {name}")
    for k in sorted(a.keys() - b.keys()):
        diffs.append(f"only in reference: {k[0]}:{k[1]} ({a[k]['cpp_type']})")
    for k in sorted(b.keys() - a.keys()):
        diffs.append(f"only in test: {k[0]}:{k[1]} ({b[k]['cpp_type']})")
    for k in sorted(a.keys() & b.keys()):
        for field in ("cpp_type", "inputs", "outputs", "config"):
            x, y = a[k].get(field), b[k].get(field)
            if field == "config" and isinstance(x, dict) and isinstance(y, dict):
                bad = sorted(f for f in x.keys() | y.keys()
                             if not _same(x.get(f), y.get(f), unverified, f"{k[0]}:{k[1]}:{f}"))
                if bad:
                    diffs.append(f"{k[0]}:{k[1]}: config fields differ: " + ", ".join(
                        f"{f}={x.get(f)!r} vs {y.get(f)!r}" for f in bad))
            elif not _same(x, y):
                diffs.append(f"{k[0]}:{k[1]}: {field} differs: {x!r} vs {y!r}")
    return diffs


def main(argv=None):
    import sys
    args = sys.argv[1:] if argv is None else argv
    if len(args) != 2:
        print("usage: python3 -m eicrecon.wiring reference.json test.json")
        return 2
    ref, test = read_wiring(args[0]), read_wiring(args[1])
    unverified = []
    diffs = compare_wiring(ref, test, unverified)
    for d in diffs:
        print(d)
    for u in unverified:
        print(f"unverified (function of the same type, state not compared): {u}")
    print(f"{len(ref)} reference wirings, {len(test)} test wirings, {len(diffs)} differences, "
          f"{len(unverified)} unverified functions")
    return 1 if diffs else 0


if __name__ == "__main__":
    raise SystemExit(main())
