#!/usr/bin/env python3
"""Verify that central tracker measurements retain a usable MC-truth path.

The check reads a persisted EICrecon output file through PODIO and follows:

Measurement2D -> TrackerHit -> RawTrackerHit
              -> MCRecoTrackerHitAssociation -> SimTrackerHit -> MCParticle

Measurements produced from digitized noise legitimately have no truth
association. They are accepted only when the raw-hit timestamp is zero, as
assigned by RandomNoisePixel.
"""

import argparse
import sys

from podio.reading import get_reader


REQUIRED_COLLECTIONS = {
    "CentralTrackerMeasurements",
    "CentralTrackingRawHitAssociations",
    "MCParticles",
}


def object_key(obj):
    """Return a stable (collectionID, index) key for a PODIO object."""
    object_id = obj.id()
    return (object_id.collectionID, object_id.index)


def require_available(obj, relation_description, event_number):
    """Fail with context when a persisted PODIO relation is unavailable."""
    if not obj.isAvailable():
        try:
            relation_id = f" (ObjectID={object_key(obj)})"
        except Exception:
            relation_id = ""
        raise RuntimeError(
            f"event {event_number}: unavailable {relation_description}"
            f"{relation_id}"
        )


def check_file(filename):
    """Check every event and return summary counters."""
    # Select ROOT TTree or RNTuple reading from the file metadata. CI writes
    # RNTuple output, while local debugging often uses the TTree backend.
    reader = get_reader(filename)
    events = reader.get("events")

    event_count = 0
    measurement_count = 0
    measurement_hit_count = 0
    truth_matched_hit_count = 0
    noise_hit_count = 0

    for event_number, event in enumerate(events):
        event_count += 1
        available_collections = set(event.getAvailableCollections())
        missing_collections = REQUIRED_COLLECTIONS - available_collections
        if missing_collections:
            missing = ", ".join(sorted(missing_collections))
            raise RuntimeError(
                f"event {event_number}: missing required collection(s): {missing}"
            )

        measurements = event.get("CentralTrackerMeasurements")
        associations = event.get("CentralTrackingRawHitAssociations")
        mc_particle_keys = {
            object_key(mc_particle) for mc_particle in event.get("MCParticles")
        }

        # One raw digitized hit may be associated with more than one simulated
        # hit. Store all associations under the raw hit's stable ObjectID.
        associations_by_raw_hit = {}
        for association_number, association in enumerate(associations):
            raw_hit = association.getRawHit()
            sim_hit = association.getSimHit()
            require_available(
                raw_hit,
                f"raw hit in CentralTrackingRawHitAssociations[{association_number}]",
                event_number,
            )
            require_available(
                sim_hit,
                f"sim hit in CentralTrackingRawHitAssociations[{association_number}]",
                event_number,
            )

            mc_particle = sim_hit.getParticle()
            require_available(
                mc_particle,
                (
                    "MCParticle reached through "
                    f"CentralTrackingRawHitAssociations[{association_number}]"
                ),
                event_number,
            )
            if object_key(mc_particle) not in mc_particle_keys:
                raise RuntimeError(
                    f"event {event_number}: MCParticle reached through "
                    "CentralTrackingRawHitAssociations"
                    f"[{association_number}] is not in MCParticles"
                )
            associations_by_raw_hit.setdefault(object_key(raw_hit), []).append(
                association
            )

        for measurement_number, measurement in enumerate(measurements):
            measurement_count += 1
            hits = measurement.getHits()
            if len(hits) == 0:
                raise RuntimeError(
                    f"event {event_number}: CentralTrackerMeasurements"
                    f"[{measurement_number}] has no TrackerHit relation"
                )

            for hit_number, hit in enumerate(hits):
                measurement_hit_count += 1
                require_available(
                    hit,
                    (
                        f"TrackerHit in CentralTrackerMeasurements"
                        f"[{measurement_number}].hits[{hit_number}]"
                    ),
                    event_number,
                )

                raw_hit = hit.getRawHit()
                require_available(
                    raw_hit,
                    (
                        f"RawTrackerHit reached from CentralTrackerMeasurements"
                        f"[{measurement_number}].hits[{hit_number}]"
                    ),
                    event_number,
                )

                hit_associations = associations_by_raw_hit.get(
                    object_key(raw_hit), []
                )
                if not hit_associations:
                    # RandomNoisePixel marks noise with timestamp zero. Do not
                    # silently classify an arbitrary unassociated hit as noise.
                    raw_hit_timestamp = raw_hit.getTimeStamp()
                    if raw_hit_timestamp != 0:
                        raise RuntimeError(
                            f"event {event_number}: unassociated RawTrackerHit "
                            f"{object_key(raw_hit)} has nonzero timestamp "
                            f"{raw_hit_timestamp}"
                        )
                    noise_hit_count += 1
                    continue

                # Association endpoints were checked while building the map.
                truth_matched_hit_count += 1

    if event_count == 0:
        raise RuntimeError("input contains no events")
    if measurement_count == 0:
        raise RuntimeError("input contains no CentralTrackerMeasurements")
    if truth_matched_hit_count == 0:
        raise RuntimeError(
            "no CentralTrackerMeasurement could be traced to an MCParticle"
        )

    return {
        "events": event_count,
        "measurements": measurement_count,
        "measurement_hits": measurement_hit_count,
        "truth_matched_hits": truth_matched_hit_count,
        "noise_hits": noise_hit_count,
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("file", help="EICrecon PODIO ROOT output file")
    args = parser.parse_args()

    try:
        counts = check_file(args.file)
    except Exception as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 1

    print(
        "PASS: central tracking truth relations are readable after PODIO "
        "serialization"
    )
    print(
        "Checked "
        f"{counts['events']} events, "
        f"{counts['measurements']} measurements, and "
        f"{counts['measurement_hits']} measurement-hit relations: "
        f"{counts['truth_matched_hits']} truth matched, "
        f"{counts['noise_hits']} timestamp-zero noise hits."
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
