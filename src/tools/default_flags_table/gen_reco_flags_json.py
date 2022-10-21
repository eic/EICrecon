import json
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('input_file', default="all_flags_dump.json",  help="Input file name")
# parser.add_argument('output_base_name', help="Output files names (no file extensions here)")
# parser.add_argument('-n', '--nevents', default="0", help="Number of events to process")
args = parser.parse_args()


reco_prefixes = [
    "B0TRK",
    "BEMC",
    "BTRK",
    "BVTX",
    "ECGEM",
    "ECTRK",
    "EEMC",
    "FOFFMTRK",
    "HCAL",
    "MPGD",
    "RPOTS",
    "ZDC",
    "Tracking",
    "Reco",
    "Digi",
    "Calorimetry"
]

# Case independent representation of prefixes to compare
low_case_reco_prefixes = [r.casefold() for r in reco_prefixes]

# Load json with all flags
all_records = []
with open(args.input_file) as f:
    all_records = json.load(f)

# Here we will save only flag records corresponding to reconstruction
dump_reco_flags = []

# Go over reco_prefixes
for reco_prefix in reco_prefixes:

    print(reco_prefix)

    # Find all flags that starts with this prefix (excluding loggings and tags)
    reco_flag_records = [r
                         for r in all_records
                         if r[0].casefold().startswith(reco_prefix.lower())
                         and 'LogLevel' not in r[0]
                         and 'InputTags' not in r[0]
                         and 'input_tags' not in r[0]
                         and 'verbose' not in r[0]]

    dump_reco_flags.extend(reco_flag_records)

# -------------------------------------------------------------
from reco_flags import eicrecon_reco_flags as user_reco_flags

# Now we have all flags from eicrecon flags dump in
print(dump_reco_flags)
# And all flags values that are set by collaboration
print(user_reco_flags)

# 1. Find all flags dump_reco_flags that have different values than user_reco_flags
# 2. All flags that appear only in user_reco_flags
# 3. All flags that appear in dump_reco_flags




