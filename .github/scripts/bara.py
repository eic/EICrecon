# SPDX-License-Identifier: MIT
# Copyright (C) 2023 Dmitry Kalinkin

import sys
from pathlib import Path

import uproot
import awkward as ak
import deepdiff
from pprint import pprint

files = sys.argv[1:]

ts = {}
prev_names = None
for f in files:
    print(f)
    ts[f] = uproot.open(f)["events"]
    names = set([b.name for b in ts[f].branches])

    if prev_names is not None:
        print(deepdiff.DeepDiff(prev_names, names))

    prev_names = names

#for b in sorted(names):
#    for l in [b.name for b in ts[f][b].branches]:
#        print(b, l)
#        #for f in files:
#        #    print(f"{f}\t", end="")
#        #print()
#        for f in files:
#            try:
#                print(f"{ak.mean(ts[f][b][l].array())}\t", end="")
#            except uproot.KeyInFileError:
#                print("-\t", end="")
#            except uproot.interpretation.identify.UnknownInterpretation:
#                print("-\t", end="")
#        print()

t1 = ts[files[0]]
t2 = ts[files[1]]
for b in sorted(names):
    if "Clust" in b: continue
    if b == "PARAMETERS": continue
    #print("b:", b)
    b1 = t1[b]
    b2 = t2[b]
    #for a1, a2 in zip(ak.unzip(t1[b].arrays()), ak.unzip(t2[b].arrays())):
    #    print(ak.mean(ak.num(a1, axis=1)))
    #    print(ak.mean(ak.num(a2, axis=1)))
    #    break

    sb1 = set([b.name for b in b1.branches])
    sb2 = set([b.name for b in b2.branches])
    deepdiff.DeepDiff(sb1, sb2)

    for sb in set.intersection(sb1, sb2):
        try:
            a1 = b1[sb].array()
            a2 = b2[sb].array()
            #print(a1)
            #print(a2)
            #print("-"*20)
            if ((ak.num(a1, axis=0) != ak.num(a2, axis=0))
              or ak.any(ak.num(a1, axis=1) != ak.num(a2, axis=1))
              or ak.any(a1 != a2)):
                diff = deepdiff.DeepDiff(a1.tolist(), a2.tolist(), ignore_nan_inequality=True)
                if True:
                #for a1, a2 in zip(ak.unzip(b1.arrays()), ak.unzip(b2.arrays())):
                    #assert a1.type == a2.type
                    #if ("Associations" not in b1.name) and not ak.all(ak.ravel(a1) == ak.ravel(a2)):
                    if diff:
                        print(sb)
                        print("NOT EQUAL")
                        pprint(diff.pretty())
                        print(a1)
                        print(a2)
        except ValueError as e:
            print(e)
            #print(a1.type, a2.type)
            continue
