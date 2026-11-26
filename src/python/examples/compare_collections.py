#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 Wouter Deconinck
"""Compare podio collections between two EICrecon output files, branch by branch.

    compare_collections.py ref.edm4eic.root test.edm4eic.root B0ECalClusters [...]
    compare_collections.py ref.edm4eic.root test.edm4eic.root --prefix B0ECal

For each collection, all data branches ('<coll>/<coll>.<member>') and relation/vector-member
branches ('_<coll>_<rel>/...') are compared exactly (bitwise for floats). Relation branches
hold (collectionID, index) pairs. Collection IDs are hashes of the collection names, so they
match whenever the referenced collections have the same names.
"""

import argparse
import sys

import awkward as ak
import uproot


def branches_for(tree, coll):
    names = []
    for key in tree.keys(recursive=True):
        top = key.split("/")[0]
        if top == coll or top.startswith(f"_{coll}_"):
            if "/" in key:  # leaves only
                names.append(key)
    return sorted(names)


def same(a, b):
    if len(a) != len(b):
        return False
    if a.ndim == 1 and not isinstance(a.layout, ak.contents.ListOffsetArray):
        return bool(ak.all(a == b))
    if not ak.all(ak.num(a, axis=1) == ak.num(b, axis=1)):
        return False
    return bool(ak.all(ak.flatten(a == b, axis=None)))


def event_order(tree):
    """Entry order sorted by event number. With nthreads > 1, events are written in completion
    order, so files are compared event by event, not entry by entry."""
    key = "EventHeader/EventHeader.eventNumber"
    if key not in tree.keys(recursive=True):
        return None
    numbers = ak.flatten(tree[key].array()).to_list()
    return sorted(range(len(numbers)), key=numbers.__getitem__)


def main():
    p = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("ref")
    p.add_argument("test")
    p.add_argument("collections", nargs="*")
    p.add_argument("--prefix", help="compare all collections starting with this prefix")
    args = p.parse_args()

    ref = uproot.open(args.ref)["events"]
    test = uproot.open(args.test)["events"]
    colls = list(args.collections)
    if args.prefix:
        tops = {k.split("/")[0] for k in ref.keys(recursive=False)}
        colls += sorted(t for t in tops if t.startswith(args.prefix))
    if ref.num_entries != test.num_entries:
        print(f"event count differs: {ref.num_entries} vs {test.num_entries}")
        return 1

    ref_order, test_order = event_order(ref), event_order(test)
    if (ref_order is None) != (test_order is None):
        print("EventHeader present in only one file; comparing entry by entry")
        ref_order = test_order = None

    def read(tree, order, branch):
        arr = tree[branch].array()
        return arr if order is None else arr[order]

    failures = 0
    for coll in colls:
        rb, tb = branches_for(ref, coll), branches_for(test, coll)
        if not rb:
            print(f"MISSING in ref : {coll}")
            failures += 1
            continue
        if rb != tb:
            print(f"BRANCHES differ: {coll}: {sorted(set(rb) ^ set(tb))}")
            failures += 1
            continue
        bad = [b for b in rb
               if not same(read(ref, ref_order, b), read(test, test_order, b))]
        n = int(ak.sum(ak.num(ref[rb[0]].array(), axis=1))) if rb else 0
        status = "IDENTICAL" if not bad else "DIFFERENT"
        print(f"{status:9s} {coll:45s} {len(rb):3d} branches, {n:6d} entries"
              + (f"  differing: {bad}" if bad else ""))
        failures += bool(bad)
    print(f"{len(colls) - failures}/{len(colls)} collections identical")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
