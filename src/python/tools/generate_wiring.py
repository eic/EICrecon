#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 Wouter Deconinck
"""Convert the C++ InitPlugin wiring of EICrecon plugins into Python modules.

The input is a wiring capture of the C++ plugins (exact tags, collections and config structs,
after all loops and conditionals in InitPlugin have run):

    eicrecon-py --use-cpp all --capture-wiring cpp.json
    python3 src/python/tools/generate_wiring.py cpp.json [PLUGIN ...]

For each plugin this writes src/python/eicrecon/plugins/<module>.py with one app.add() per
wiring. Config fields equal to the struct defaults are omitted. Other values are written as the
C++ initializer expression translated to Python (e.g. `5.0 * u.MeV`), if the plugin source has
one that evaluates to exactly the captured value, and as an exact literal otherwise. Run it in
the eicrecon environment (it imports the binding modules for the config defaults).

Correctness does not depend on the expression recovery: the equivalence test compares the
wirings of the generated modules with the C++ capture, field by field.
"""

import json
import math
import os
import re
import struct
import sys

HAND_WRITTEN = {"B0ECAL", "FEMC", "FHCAL", "MPGD", "DRICH", "RICH-IRT"}

# Python forms of std::function config values (constructors in the binding modules' *_extra.h)
FUNCTION_VALUES = {
    ("DIRC", "DIRCSurfaceHits", "function"):
        'T.GeometrySplit_SimTrackerHit([[0], [1]], "DIRCHits", ["surface"])',
    ("LOWQ2", "TaggerTrackerSplitHits", "function"):
        'T.GeometrySplit_TrackerHit([[m, l] for m in [1, 2] for l in [0, 1, 2, 3]], '
        '"TaggerTrackerHits", ["module", "layer"])',
    ("beam", "BeamParticles", "function"):
        "T.ValueSplit_GeneratorStatus_PDG("
        "[[4, 11], [4, 2212], [4, 2112], [1, 11], [1, 2212], [1, 2112]])",
    ("particle_flow", "EndcapPBarrelEndcapNCalorimeterTrackProjections", "function"):
        "T.TrackPointSystemSplit([103, 113, 101, 111, 102, 116, 115])",
}


def module_name(plugin):
    return re.sub(r"\W", "_", plugin)


def plugin_source(plugin):
    """Source file with the InitPlugin of `plugin` (plugin names come from CMakeLists.txt)"""
    sys.path.insert(0, os.path.dirname(__file__))
    from generate_bindings import plugin_name, plugin_sources
    for path, _ in plugin_sources():
        if plugin_name(path) == plugin:
            return os.path.relpath(path)
    return None


# -- C++ expression recovery -----------------------------------------------------------------


def strip_comments(text):
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    return re.sub(r"//[^\n]*", "", text)


def balanced(text, start, open_ch, close_ch):
    """Index just after the bracket matching text[start] == open_ch"""
    depth, i, in_str = 0, start, False
    while i < len(text):
        c = text[i]
        if in_str:
            if c == "\\":
                i += 1
            elif c == '"':
                in_str = False
        elif c == '"':
            in_str = True
        elif c == open_ch:
            depth += 1
        elif c == close_ch:
            depth -= 1
            if depth == 0:
                return i + 1
        i += 1
    raise ValueError("unbalanced")


def split_top(text, sep=","):
    """Split on `sep` at bracket depth 0, outside strings"""
    parts, depth, cur, in_str, i = [], 0, "", False, 0
    while i < len(text):
        c = text[i]
        if in_str:
            cur += c
            if c == "\\":
                cur += text[i + 1]
                i += 1
            elif c == '"':
                in_str = False
        elif c == '"':
            in_str = True
            cur += c
        elif c in "([{":
            depth += 1
            cur += c
        elif c in ")]}":
            depth -= 1
            cur += c
        elif c == sep and depth == 0:
            parts.append(cur)
            cur = ""
        else:
            cur += c
        i += 1
    if cur.strip():
        parts.append(cur)
    return [p.strip() for p in parts]


def designated_initializers(text):
    """{.a = x, .b{y}} -> {'a': 'x', 'b': '{y}'} (top level only)"""
    out = {}
    inner = text.strip()
    if not (inner.startswith("{") and inner.endswith("}")):
        return out
    for part in split_top(inner[1:-1]):
        m = re.match(r"\.(\w+)\s*(=\s*(.*)|(\{.*\}))$", part, re.S)
        if m:
            out[m.group(1)] = (m.group(3) if m.group(3) is not None else m.group(4)).strip()
    return out


def wiring_calls(source):
    """{tag literal: {field: C++ expression}} for JOmniFactoryGeneratorT(...) calls"""
    text = strip_comments(source)
    calls = {}
    for m in re.finditer(r"JOmniFactoryGeneratorT\s*<", text):
        i = balanced(text, m.end() - 1, "<", ">")
        paren = text.find("(", i)
        if paren < 0 or text[i:paren].strip():
            continue
        args = split_top(text[paren + 1:balanced(text, paren, "(", ")") - 1])
        if len(args) >= 4 and re.fullmatch(r'"[^"]*"', args[0]):
            calls[args[0][1:-1]] = designated_initializers(args[3])
    return calls


CONSTANT = re.compile(
    r"^\s*(?:static\s+)?(?:const(?:expr)?\s+)?"
    r"(?:decltype\([\w:]+\)|double|float|int|unsigned(?:\s+(?:int|long))*|long(?:\s+long)?|"
    r"std::size_t|std::string|auto)\s+(\w+)\s*=\s*([^;]+);", re.M)


def constants(source):
    """[(name, C++ expression)] of simple local constants, in source order"""
    body = strip_comments(source)
    body = body[body.find("InitPlugin"):]
    return [(m.group(1), " ".join(m.group(2).split())) for m in CONSTANT.finditer(body)]


def translate(expr):
    """C++ expression -> Python expression, or None if outside the supported subset"""
    e = " ".join(expr.split())
    if re.search(r"static_cast|std::format|\?|->|\bnew\b", e):
        return None
    e = re.sub(r"\bstd::(sqrt|pow|exp|log|atan|tan|sin|cos|abs)\(", r"\1(", e)
    e = re.sub(r"\bstd::string\((\"[^\"]*\")\)", r"\1", e)
    e = re.sub(r"\bedm4eic::unit::(\w+)", r"eu.\1", e)
    e = re.sub(r"\bdd4hep::(\w+)", r"u.\1", e)
    e = re.sub(r"\bM_PI\b", "pi", e)
    e = re.sub(r"\btrue\b", "True", e)
    e = re.sub(r"\bfalse\b", "False", e)
    e = re.sub(r"(?<![\w.])((?:\d+\.?\d*|\.\d+)(?:[eE][+-]?\d+)?)(?:ULL|ull|UL|ul|LL|ll|U|u|L|l)\b", r"\1", e)
    if re.search(r"(?<![\w.])(?:\d+\.?\d*|\.\d+)(?:[eE][+-]?\d+)?[fF]\b", e):
        return None  # float literals: let the exact literal win
    if "::" in e or "->" in e or "&" in e or "<" in e or ">" in e:
        return None
    e = e.replace("{", "[").replace("}", "]")
    return e


# -- values --------------------------------------------------------------------------------


def as_float32(x):
    return struct.unpack("f", struct.pack("f", x))[0]


def literal(value):
    """Exact Python literal; floats that are float32 values get their short float32 form"""
    if isinstance(value, bool) or value is None:
        return repr(value)
    if isinstance(value, float):
        if math.isnan(value):
            return 'float("nan")'
        if math.isinf(value):
            return 'float("inf")' if value > 0 else '-float("inf")'
        if as_float32(value) == value:
            for digits in range(1, 10):
                short = float(f"{value:.{digits}g}")
                if as_float32(short) == value:
                    if short == value:
                        return repr(short)
                    break
        return repr(value)
    if isinstance(value, list):
        return "[" + ", ".join(literal(v) for v in value) + "]"
    if isinstance(value, dict):
        if set(value) == {"cpp_function", "description"}:
            return None  # std::function: cannot be written in Python
        return "{" + ", ".join(f"{k!r}: {literal(v)}" for k, v in value.items()) + "}"
    return repr(value)


def same(a, b, float32=False):
    if isinstance(a, bool) or isinstance(b, bool):
        return type(a) is type(b) and a == b
    if isinstance(a, (int, float)) and isinstance(b, (int, float)):
        if isinstance(b, float) and math.isnan(b):
            return isinstance(a, float) and math.isnan(a)
        if isinstance(a, int) != isinstance(b, int) and not isinstance(b, float):
            return False
        return a == b or (float32 and as_float32(float(a)) == b)
    if isinstance(a, (list, tuple)) and isinstance(b, list):
        return len(a) == len(b) and all(same(x, y, float32) for x, y in zip(a, b))
    return type(a) is type(b) and a == b


def value_source(cpp_expr, value, env):
    """Python source for a config value: translated expression if exact, else literal"""
    lit = literal(value)
    if cpp_expr is None or lit is None:
        return lit
    py = translate(cpp_expr)
    if py is None:
        return lit
    try:
        got = eval(py, dict(env))  # noqa: S307 - expression from our own sources
    except Exception:
        return lit
    float32 = isinstance(value, float) and as_float32(value) == value
    if same(got, value, float32) and py != lit:
        return py
    return lit


# -- module generation -----------------------------------------------------------------------


HEADER = '''# SPDX-License-Identifier: LGPL-3.0-or-later
"""{plugin} wiring, converted from {source} by src/python/tools/generate_wiring.py.

Values use dd4hep units (u, as in the C++ wiring) or edm4eic units (eu).
"""

from math import atan, cos, exp, log, pi, sin, sqrt, tan  # noqa: F401

from ..units import dd4hep as u  # noqa: F401
from ..units import edm4eic as eu  # noqa: F401
from . import bindings

PLUGIN = "{plugin}"


def configure(app, plugin=PLUGIN):
    T = bindings("{module}")
'''


def env_for(source):
    import eicrecon.units as units
    env = {"u": units.dd4hep, "eu": units.edm4eic, "sqrt": math.sqrt, "pi": math.pi,
           "exp": math.exp, "log": math.log, "atan": math.atan, "tan": math.tan,
           "sin": math.sin, "cos": math.cos, "abs": abs, "pow": pow, "__builtins__": {}}
    consts = []
    for name, cpp in constants(source):
        py = translate(cpp)
        if py is None:
            continue
        try:
            env[name] = eval(py, dict(env))  # noqa: S307
            consts.append((name, py))
        except Exception:
            pass
    return env, consts


def generate(plugin, records):
    from eicrecon.plugins import bindings
    module = module_name(plugin)
    T = bindings(module)
    path = plugin_source(plugin)
    source = open(path).read() if path else ""
    calls = wiring_calls(source)
    env, consts = env_for(source)
    body, used = [], set()
    # source order where the tag is a literal; others (built in loops) keep capture order
    def position(r):
        i = source.find(f'"{r["tag"]}"')
        return i if i >= 0 else len(source)
    for r in sorted(records, key=position):
        ftype = T.types.get(r["cpp_type"])
        if ftype is None:
            raise SystemExit(f"{plugin}: no binding for {r['cpp_type']}")
        defaults = ftype.config_defaults()
        exprs = calls.get(r["tag"], {})
        args = []
        for field, value in (r["config"] or {}).items():
            if same(_py(defaults.get(field)), value):
                continue
            src = FUNCTION_VALUES.get((plugin, r["tag"], field)) or value_source(
                exprs.get(field), value, env)
            if src is None:
                raise SystemExit(f"{plugin}:{r['tag']}: {field} has no Python form; port by hand")
            used.update(n for n, _ in consts if re.search(rf"\b{n}\b", src))
            args.append(f"{field}={src}")
        call = [f"app.add(T.{ftype.name}, {r['tag']!r},",
                f"        {r['inputs']!r},",
                f"        {r['outputs']!r}" + ("," if args else ")")]
        for i, a in enumerate(args):
            call.append(f"        {a}" + ("," if i < len(args) - 1 else ")"))
        body.append(call)
    out = HEADER.format(plugin=plugin, source=path, module=module)
    for name, py in consts:
        if name in used:
            out += f"    {name} = {py}\n"
    if used:
        out += "\n"
    out += "    with app.plugin(plugin):\n"
    for call in body:
        out += "\n".join("        " + line for line in call) + "\n"
    return module, out


def _py(value):
    from eicrecon.wiring import _plain
    return _plain(value)


def main(argv):
    if not argv:
        print(__doc__)
        return 2
    records = json.load(open(argv[0]))
    only = set(argv[1:])
    by_plugin = {}
    for r in records:
        by_plugin.setdefault(r["plugin"], []).append(r)
    os.makedirs("src/python/eicrecon/plugins", exist_ok=True)
    for plugin, recs in sorted(by_plugin.items()):
        if plugin in HAND_WRITTEN and plugin not in only:
            continue
        if plugin_source(plugin) is None or (only and plugin not in only):
            continue
        module, text = generate(plugin, recs)
        with open(f"src/python/eicrecon/plugins/{module}.py", "w") as f:
            f.write(text)
        print(f"{plugin:16s}: {len(recs):3d} wirings -> eicrecon/plugins/{module}.py")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
