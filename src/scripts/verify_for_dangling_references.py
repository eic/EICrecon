#!/usr/bin/env python3
"""Verify that every collectionID referenced by relation branches
   (_<Coll>_<relation>.collectionID) in the 'events' tree exists in the
   podio_metadata events CollectionTypeInfo table."""
import sys
import re
import argparse
import numpy as np
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


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("file", help="podio ROOT file")
    args = ap.parse_args()

    f = uproot.open(args.file)
    events = f["events"]
    meta = f["podio_metadata"]

    known_ids = set(
        int(x) for x in
        meta["events___CollectionTypeInfo/events___CollectionTypeInfo.collectionID"]
            .array(library="np")[0]
    )
    pattern = re.compile(r"^_.+_.+\.collectionID$")
    branches = [k for k in events.keys() if pattern.match(k)]

    errors = 0
    checked = 0
    for br in branches:
        arr = events[br].array(library="np")
        flat = np.concatenate([np.asarray(x, dtype=np.uint32) for x in arr]) if len(arr) else np.array([], dtype=np.uint32)
        unique_ids = np.unique(flat)
        checked += 1
        unknown = [int(i) for i in unique_ids if int(i) not in (0, 0xFFFFFFFF) and int(i) not in known_ids]
        if unknown:
            # Branch key looks like '_Foo_bar/_Foo_bar.collectionID' — take prefix.
            short = br.split("/", 1)[0]
            if short in KNOWN_ISSUES:
                print(f"KNOWN: branch {br} references unknown collectionID(s): {unknown}")
            else:
                errors += 1
                print(f"ERROR: branch {br} references unknown collectionID(s): {unknown}")

    print(f"\nChecked {checked} relation branches; {errors} unexpected error(s) (known issues excluded).")
    sys.exit(1 if errors else 0)


if __name__ == "__main__":
    main()
