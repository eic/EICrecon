#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 Wouter Deconinck
"""Compare podio collections between two EICrecon output files, branch by branch.

    compare_collections.py ref.edm4eic.root test.edm4eic.root B0ECalClusters [...]
    compare_collections.py ref.edm4eic.root test.edm4eic.root --prefix B0ECal
    compare_collections.py ref.edm4eic.root test.edm4eic.root --all

Both podio backends are supported (podio:output_backend=root or rntuple), also mixed. For each
collection, all leaves of its data ('<coll>.<member>'), relations and vector members
('_<coll>_<name>[.<member>]') are compared bitwise (NaN equals NaN). Leaves are named as in an
RNTuple; in a TTree, '<coll>.<member>' is the branch '<coll>/<coll>.<member>'. Relations hold
(collectionID, index) pairs. Collection IDs are hashes of the collection names, so they match
whenever the referenced collections have the same names.
"""

import argparse
import re
import sys

import awkward as ak
import numpy as np
import uproot


class Events:
    """The events of a podio file (TTree or RNTuple), as leaves grouped by collection."""

    def __init__(self, path):
        self.tree = uproot.open(path)["events"]
        self.num_entries = self.tree.num_entries
        # TTree: '<top>/<top>.<member>' or a leaf branch '<top>'; RNTuple: '<top>[.<member>...]'
        rntuple = "RNTuple" in getattr(self.tree, "classname", "")
        sep = "." if rntuple else "/"
        keys = self.tree.keys(recursive=True)
        parents = {k.rsplit(sep, 1)[0] for k in keys if sep in k}
        self.leaves = {}  # top-level name -> {leaf name: key}
        for key in keys:
            if key in parents:
                continue
            top = key.split(sep)[0]
            # TTree leaves of fixed-size arrays are named '<member>[<size>]'
            name = key if rntuple else re.sub(r"\[\d+\]$", "", key.rsplit("/", 1)[-1])
            self.leaves.setdefault(top, {})[name] = key

    def collections(self):
        """Collection names (not relation/vector-member or global-parameter branches)"""
        return {t for t in self.leaves if not t.startswith("_") and not t.startswith("GP")}

    def leaves_for(self, coll):
        """{leaf name: key} of a collection, including its relations and vector members"""
        result = {}
        for top, leaves in self.leaves.items():
            if top == coll or top.startswith(f"_{coll}_"):
                result.update(leaves)
        return result

    def array(self, key):
        return self.tree[key].array()


def same(a, b):
    """Bitwise equality, so that NaN equals NaN (same payload) and -0.0 differs from 0.0"""
    if len(a) != len(b):
        return False
    if a.ndim > 1 or isinstance(a.layout, ak.contents.ListOffsetArray):
        if not ak.all(ak.num(a, axis=1) == ak.num(b, axis=1)):
            return False
        a, b = ak.flatten(a, axis=None), ak.flatten(b, axis=None)
    x, y = ak.to_numpy(a), ak.to_numpy(b)
    if x.dtype != y.dtype:
        return False
    if x.dtype.kind == "f":
        x, y = x.view(f"u{x.dtype.itemsize}"), y.view(f"u{y.dtype.itemsize}")
    return bool(np.array_equal(x, y))


def event_order(events):
    """Entry order sorted by (run number, event number). With nthreads > 1, events are written
    in completion order, so files are compared event by event, not entry by entry."""
    leaves = events.leaves_for("EventHeader")
    names = [f"EventHeader.{m}" for m in ("runNumber", "eventNumber")]
    if not all(n in leaves for n in names):
        return None
    runs, numbers = (ak.flatten(events.array(leaves[n])).to_list() for n in names)
    return sorted(range(len(numbers)), key=lambda i: (runs[i], numbers[i]))


def main():
    p = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("ref")
    p.add_argument("test")
    p.add_argument("collections", nargs="*")
    p.add_argument("--prefix", help="compare all collections starting with this prefix")
    p.add_argument("--all", action="store_true",
                   help="compare all collections in either file (a collection missing from "
                        "one file is a failure)")
    args = p.parse_args()

    ref = Events(args.ref)
    test = Events(args.test)
    colls = list(args.collections)
    if args.prefix:
        colls += sorted(t for t in ref.collections() if t.startswith(args.prefix))
    if args.all:
        only = ref.collections() ^ test.collections()
        for name in sorted(only):
            print(f"ONLY IN {'ref ' if name in ref.collections() else 'test'}: {name}")
        colls += sorted(ref.collections() & test.collections())
        if only:
            print(f"{len(only)} collections present in only one file")
            return 1
    if ref.num_entries != test.num_entries:
        print(f"event count differs: {ref.num_entries} vs {test.num_entries}")
        return 1

    ref_order, test_order = event_order(ref), event_order(test)
    if (ref_order is None) != (test_order is None):
        print("EventHeader present in only one file; comparing entry by entry")
        ref_order = test_order = None

    def read(events, order, key):
        arr = events.array(key)
        return arr if order is None else arr[order]

    failures = 0
    for coll in colls:
        rl, tl = ref.leaves_for(coll), test.leaves_for(coll)
        if not rl:
            print(f"MISSING in ref : {coll}")
            failures += 1
            continue
        if rl.keys() != tl.keys():
            print(f"LEAVES differ  : {coll}: {sorted(rl.keys() ^ tl.keys())}")
            failures += 1
            continue
        names = sorted(rl)
        bad = [n for n in names
               if not same(read(ref, ref_order, rl[n]), read(test, test_order, tl[n]))]
        n = int(ak.sum(ak.num(ref.array(rl[names[0]]), axis=1)))
        status = "IDENTICAL" if not bad else "DIFFERENT"
        print(f"{status:9s} {coll:45s} {len(names):3d} leaves,   {n:6d} entries"
              + (f"  differing: {bad}" if bad else ""))
        failures += bool(bad)
    print(f"{len(colls) - failures}/{len(colls)} collections identical")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
