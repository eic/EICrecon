#!/usr/bin/env python3
"""Verify that every collectionID referenced by relation branches
   (_<Coll>_<relation>.collectionID) in the 'events' tree exists in the
   podio_metadata events CollectionTypeInfo table."""
import sys
import re
import argparse
import numpy as np
import awkward as ak
import uproot


# Branches with known dangling collectionID references. These are still
# reported (as KNOWN) but do not contribute to the non-zero exit code.
KNOWN_ISSUES = {
    "_DRICHAerogelIrtCherenkovParticleID_chargedParticle",
    "_DRICHGasIrtCherenkovParticleID_chargedParticle",
    "_DRICHRawHitsAssociations_simHit",
    "_DRICHRawHitsLinks_to",
    "_EcalBarrelImagingClusters_clusters",
    "_EcalBarrelTruthClusters_clusters",
    "_EcalEndcapNClusters_particleIDs",
    "_EcalEndcapNExpectedClusters_particleIDs",
    "_EcalEndcapNRemnantClusters_particleIDs",
    "_HcalEndcapPInsertClusters_hits",
    "_HcalEndcapPInsertRemnantClusters_hits",
    "_RICHEndcapNRawHitsAssociations_simHit",
    "_RICHEndcapNRawHitsLinks_to",
    "_TOFBarrelClusterHits_hits",
    "_TOFEndcapClusterHits_hits",
}


def read_collection_id_field(obj, group):
    """Read the collectionID sub-field of a podio relation group.

    Works for both TTree- and RNTuple-backed containers. Returns a flat
    numpy array of uint32 values.
    """
    a = obj[group].arrays()
    if group in a.fields:
        # RNTuple: nested record.
        arr = a[group].collectionID
    else:
        # TTree: dotted flat field.
        arr = a[f"{group}.collectionID"]
    return np.asarray(ak.flatten(arr, axis=None), dtype=np.uint32)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("file", help="podio ROOT file")
    args = ap.parse_args()

    f = uproot.open(args.file)
    events = f["events"]
    meta = f["podio_metadata"]

    known_ids = set(int(x) for x in read_collection_id_field(meta, "events___CollectionTypeInfo"))

    # Collect unique relation group names, e.g. '_TOFBarrelClusterHits_hits'.
    pattern = re.compile(r"^(_[^/]+_[^/.]+)(?:/\1)?\.collectionID$")
    groups = sorted({m.group(1) for k in events.keys() for m in [pattern.match(k)] if m})

    errors = 0
    for group in groups:
        ids = read_collection_id_field(events, group)
        unique_ids = np.unique(ids)
        unknown = [int(i) for i in unique_ids if int(i) not in (0, 0xFFFFFFFF) and int(i) not in known_ids]
        if unknown:
            if group in KNOWN_ISSUES:
                print(f"KNOWN: branch {group} references unknown collectionID(s): {unknown}")
            else:
                errors += 1
                print(f"ERROR: branch {group} references unknown collectionID(s): {unknown}")

    print(f"\nChecked {len(groups)} relation branches; {errors} unexpected error(s) (known issues excluded).")
    sys.exit(1 if errors else 0)


if __name__ == "__main__":
    main()
