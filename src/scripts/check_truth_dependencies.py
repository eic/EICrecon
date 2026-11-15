#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026, ePIC Collaboration
"""
Static analysis of JOmniFactory wiring to find which output collections cannot be
produced when reconstructing from digitization-only input (i.e. without MCParticles,
simulated hits, or truth associations).

The script parses `JOmniFactoryGeneratorT<...>(tag, {inputs}, {outputs}, ...)` calls in
plugin sources and the input declarations (`PodioInput<T[, true]>`,
`VariadicPodioInput<T[, true]>`, `Input<T[, true]>`) in factory headers, and then
determines which collections are producible from a given set of available collections.

Limitations: only wirings with literal string tags are understood. Wirings built from
variables or loops are reported as unparsed, and their outputs are assumed producible.
"""

import argparse
import re
import sys
from collections import defaultdict
from pathlib import Path

GEN_RE = re.compile(r"JOmniFactoryGeneratorT\s*<")
STR_RE = re.compile(r'"([^"]*)"')
# Collections that are truth by design (outputs) or simulation inputs
TRUTH_RE = re.compile(r"(Links?|Associations?|Truth|^MC|^Generated|HitContributions)")
SIM_RE = re.compile(r"Hits$")
INPUT_DECL_RE = re.compile(
    r"\b(Variadic)?(?:Podio)?Input\s*<\s*([^;{]*?)>\s*(\w+)\s*\{", re.S
)


def match_balanced(text, start, open_ch, close_ch):
    """Return index just past the bracket matching text[start] (== open_ch)."""
    depth = 0
    i = start
    in_str = False
    while i < len(text):
        c = text[i]
        if in_str:
            if c == "\\":
                i += 1
            elif c == '"':
                in_str = False
        elif c == '"':
            in_str = True
        elif c == "/" and text.startswith("//", i):
            i = text.find("\n", i)
            if i < 0:
                return len(text)
            continue
        elif c == open_ch:
            depth += 1
        elif c == close_ch and not (close_ch == ">" and i > 0 and text[i - 1] == "-"):
            depth -= 1
            if depth == 0:
                return i + 1
        i += 1
    return len(text)


def split_top_level(text):
    """Split a comma-separated argument list at top level (ignoring nested brackets)."""
    parts, depth, cur, in_str = [], 0, [], False
    i = 0
    while i < len(text):
        c = text[i]
        if in_str:
            cur.append(c)
            if c == "\\":
                cur.append(text[i + 1])
                i += 1
            elif c == '"':
                in_str = False
        elif c == '"':
            in_str = True
            cur.append(c)
        elif c == "/" and text.startswith("//", i):
            j = text.find("\n", i)
            i = len(text) if j < 0 else j
            continue
        elif c in "([{<":
            depth += 1
            cur.append(c)
        elif c in ")]}>":
            depth -= 1
            cur.append(c)
        elif c == "," and depth == 0:
            parts.append("".join(cur).strip())
            cur = []
        else:
            cur.append(c)
        i += 1
    if "".join(cur).strip():
        parts.append("".join(cur).strip())
    return parts


def string_list(arg):
    """Parse `{"a", "b"}` into a list of strings; None if non-literal entries exist."""
    arg = arg.strip()
    if not (arg.startswith("{") and arg.endswith("}")):
        return None
    inner = arg[1:-1]
    entries = [e for e in split_top_level(inner) if e]
    out = []
    for e in entries:
        m = re.fullmatch(r'\s*"([^"]*)"\s*', e)
        if not m:
            return None
        out.append(m.group(1))
    return out


def parse_wirings(src_dirs):
    wirings, unparsed = [], []
    for d in src_dirs:
        for path in sorted(Path(d).rglob("*.cc")):
            text = path.read_text(errors="replace")
            for m in GEN_RE.finditer(text):
                tmpl_end = match_balanced(text, m.end() - 1, "<", ">")
                factory = re.sub(r"\s+", "", text[m.end() : tmpl_end - 1])
                j = tmpl_end
                while j < len(text) and text[j].isspace():
                    j += 1
                if j >= len(text) or text[j] != "(":
                    continue
                args_end = match_balanced(text, j, "(", ")")
                args = split_top_level(text[j + 1 : args_end - 1])
                line = text.count("\n", 0, m.start()) + 1
                loc = f"{path}:{line}"
                if len(args) < 3:
                    unparsed.append((loc, factory, "too few arguments", []))
                    continue
                tag = re.fullmatch(r'\s*"([^"]*)"\s*', args[0])
                ins, outs = string_list(args[1]), string_list(args[2])
                if tag is None or ins is None or outs is None:
                    # keep any literal output names, so that they are not reported as missing
                    literal_outs = STR_RE.findall(args[2]) if outs is None else outs
                    unparsed.append((loc, factory, "non-literal names", literal_outs))
                    continue
                wirings.append(
                    {"loc": loc, "factory": factory, "tag": tag.group(1), "inputs": ins,
                     "outputs": outs}
                )
    return wirings, unparsed


def factory_base_name(factory):
    name = factory.split("<", 1)[0]
    return name.split("::")[-1]


def find_factory_inputs(name, src_root, cache):
    """Return list of (is_variadic, is_optional) for the factory's inputs, in order."""
    if name in cache:
        return cache[name]
    candidates = list(Path(src_root).rglob(f"{name}.h"))
    if not candidates:
        # factory may be defined in a header with a different name
        for p in Path(src_root).rglob("*.h"):
            if re.search(rf"\bclass\s+{re.escape(name)}\b", p.read_text(errors="replace")):
                candidates.append(p)
                break
    decls = None
    for p in candidates:
        text = p.read_text(errors="replace")
        cls = re.search(rf"\bclass\s+{re.escape(name)}\b", text)
        if not cls:
            continue
        body = text[cls.start() :]
        decls = []
        for m in INPUT_DECL_RE.finditer(body):
            variadic = m.group(1) is not None
            targs = split_top_level(m.group(2))
            optional = len(targs) > 1 and targs[-1].strip() in ("true", "IsOptional")
            if targs[-1].strip() == "IsOptional":
                optional = None  # depends on template parameter
            decls.append((variadic, optional))
        break
    cache[name] = decls
    return decls


def assign_inputs(wiring, decls):
    """Return list of (collection, optional) for a wiring given input declarations."""
    ins = wiring["inputs"]
    if decls is None:
        return [(c, False) for c in ins]
    factory = wiring["factory"]
    tmpl_optional = bool(re.search(r",\s*true\s*>$", factory))
    resolved = [(v, tmpl_optional if o is None else o) for v, o in decls]
    n_fixed = sum(1 for v, _ in resolved if not v)
    n_var = len(ins) - n_fixed
    out, i = [], 0
    for variadic, optional in resolved:
        if variadic:
            for _ in range(max(n_var, 0)):
                if i < len(ins):
                    out.append((ins[i], optional))
                    i += 1
        else:
            if i < len(ins):
                out.append((ins[i], optional))
                i += 1
    # any extra inputs (e.g. inputs declared via helpers we did not parse)
    out.extend((c, False) for c in ins[i:])
    return out


def default_output_collections(path):
    text = Path(path).read_text(errors="replace")
    start = text.find("output_collections = {")
    end = match_balanced(text, text.find("{", start), "{", "}")
    return STR_RE.findall(text[start:end])


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--src", default="src", help="source root")
    parser.add_argument(
        "--available",
        default=(
            r"EventHeader,(?!EcalBarrelScFi).*RawHits,.*PulsesWithNoise,.*ADCTDC,"
            r"MCBeamElectronsCloned,MCBeamHadronsCloned"
        ),
        help="comma-separated regexes of collections available in the input file",
    )
    parser.add_argument(
        "--check",
        action="store_true",
        help="exit with non-zero status if reconstruction outputs are not producible",
    )
    args = parser.parse_args()

    src = Path(args.src)
    wirings, unparsed = parse_wirings([src / "global", src / "detectors"])
    cache = {}
    for w in wirings:
        w["resolved"] = assign_inputs(w, find_factory_inputs(factory_base_name(w["factory"]),
                                                             src, cache))

    producers = defaultdict(list)
    for w in wirings:
        for o in w["outputs"]:
            producers[o].append(w)

    # Available collections: those matching the regexes and produced by some factory or
    # well-known input collections
    patterns = [re.compile(p + "$") for p in args.available.split(",") if p]
    all_names = set(producers) | {c for w in wirings for c in w["inputs"]}
    available = {c for c in all_names if any(p.match(c) for p in patterns)}
    available |= {p.pattern[:-1] for p in patterns if re.fullmatch(r"[\w]+\$", p.pattern)}

    # Outputs of unparsed wirings are unknown; assume they are producible
    unknown = {o for *_, outs in unparsed for o in outs}
    # Outputs of unparsed wirings with non-literal names (e.g. built in loops) cannot be
    # matched at all; treat inputs that no parsed wiring produces but that look like
    # reconstruction collections as unknown when they appear in an unparsed wiring source file
    unparsed_files = {loc.rsplit(":", 1)[0] for loc, *_ in unparsed}
    for w in wirings:
        if w["loc"].rsplit(":", 1)[0] in unparsed_files:
            for c, _ in w["resolved"]:
                if c not in producers and not SIM_RE.search(c) and c != "MCParticles":
                    unknown.add(c)

    producible = set(available)
    changed = True
    while changed:
        changed = False
        for w in wirings:
            if all(o in producible for o in w["outputs"]):
                continue
            if all(c in producible or c in unknown for c, opt in w["resolved"] if not opt):
                for o in w["outputs"]:
                    if o not in producible:
                        producible.add(o)
                        changed = True

    # For each non-producible factory, the missing required inputs
    blocked = []
    for w in wirings:
        if all(o in producible for o in w["outputs"]):
            continue
        missing = [c for c, opt in w["resolved"] if not opt and c not in producible]
        blocked.append((w, missing))

    # Root causes: blocked factories where all missing inputs have no producer at all
    # (i.e. truth/simulation collections), or are produced only by other blocked factories
    sources = {c for c in all_names if c not in producers}
    roots = [(w, [m for m in miss if m in sources]) for w, miss in blocked]
    roots = [(w, m) for w, m in roots if m]

    defaults = default_output_collections(src / "services/io/podio/JEventProcessorPODIO.cc")
    missing_defaults = [
        c for c in defaults if c in producers and c not in producible and c not in unknown
    ]
    # Pre-digitization collections: upstream of collections available in the input file
    # (i.e. simulation-level intermediates that are not expected to be available)
    predigi = set()
    stack = [c for c in available if c in producers]
    while stack:
        c = stack.pop()
        for w in producers[c]:
            for i in w["inputs"]:
                if i not in predigi and i not in available:
                    predigi.add(i)
                    stack.append(i)
    missing_predigi = [c for c in missing_defaults if c in predigi]
    missing_truth = [c for c in missing_defaults if c not in predigi and TRUTH_RE.search(c)]
    missing_reco = [c for c in missing_defaults if c not in predigi and not TRUTH_RE.search(c)]

    def p(*a):
        print(*a)

    p(f"# Truth dependency analysis (available: {args.available})\n")
    p(f"Parsed {len(wirings)} wirings, {len(unparsed)} unparsed.\n")
    p("## Factories with non-truth outputs requiring a collection that no factory produces\n")
    p("| Factory | Tag | Missing required inputs | Location |")
    p("|---|---|---|---|")
    roots = [
        (w, m) for w, m in roots
        if any(
            o not in producible and o not in predigi and not TRUTH_RE.search(o)
            for o in w["outputs"]
        )
    ]
    for w, m in sorted(roots, key=lambda x: (factory_base_name(x[0]["factory"]), x[0]["tag"])):
        loc = w["loc"].split("src/", 1)[-1]
        p(f"| `{factory_base_name(w['factory'])}` | `{w['tag']}` | {', '.join(f'`{x}`' for x in m)} | `{loc}` |")
    p("")
    blocked_by = {id(w): miss for w, miss in blocked}

    def why(c):
        return ", ".join(f"`{x}`" for x in sorted({m for w in producers[c] for m in blocked_by.get(id(w), [])}))

    p("## Default output collections that are not producible (reconstruction)\n")
    for c in missing_reco:
        p(f"- `{c}` (missing: {why(c)})")
    p("")
    p("## Default output collections that are not producible (truth by design)\n")
    for c in missing_truth:
        p(f"- `{c}` (missing: {why(c)})")
    p("")
    p("## Default output collections that are not producible (pre-digitization)\n")
    for c in missing_predigi:
        p(f"- `{c}` (missing: {why(c)})")
    p("")
    p("## Unparsed wirings (outputs not analyzed)\n")
    for loc, factory, reason, _ in unparsed:
        p(f"- `{loc.split('src/', 1)[-1]}` `{factory_base_name(factory)}`: {reason}")
    if args.check and (missing_reco or roots):
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
