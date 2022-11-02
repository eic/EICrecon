import json
import argparse
import os
import re

import numpy as np
# from deepdiff import DeepDiff

parser = argparse.ArgumentParser()
parser.add_argument('input_file', default="all_flags_dump.json", nargs="?", help="Input file name")
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

print("---------------------------\n DUMP FLAGS\n---------------------------")
for i in range(10):
    record = dump_reco_flags[i]
    print("{:<60} {:<30}".format(record[0], record[1]))

print("---------------------------\n USER FLAGS\n---------------------------")
for i in range(10):
    record = user_reco_flags[i]
    print("{:<60} {:<30}".format(record[0], record[1]))

print("---------------------------\n Difference\n---------------------------")

# for value in dump_reco_flags:
#     dump_flag_names.append(value[0])
# print(dump_flag_names)
#
# for value in user_reco_flags:
#     user_flag_names.append(value[0])
# print(user_flag_names)
#
# for name in dump_flag_names:
#     if name not in user_flag_names:
#         dump_names_only.append(name)
# print(dump_names_only)
#
# for name in user_flag_names:
#     if name not in dump_flag_names:
#         user_names_only.append(name)
# print(user_names_only)


dump_flag_names = [value[0] for value in dump_reco_flags]
user_flag_names = [value[0] for value in user_reco_flags]
dump_names_only = [name for name in dump_flag_names if name not in user_flag_names]
user_names_only = [name for name in user_flag_names if name not in dump_flag_names]

dump_name_values = {}
for r in dump_reco_flags:
    dump_name_values[r[0]] = r[1]

comparison_table = "" #"<table>\n"

for user_reco_flag in user_reco_flags:
    to_compare_user_flag_name = user_reco_flag[0]
    to_compare_user_flag_value = user_reco_flag[1]
    if to_compare_user_flag_name in dump_name_values:
        to_compare_dump_flag_value = dump_name_values[to_compare_user_flag_name]
        if to_compare_dump_flag_value != to_compare_user_flag_value:
            print(f"{to_compare_user_flag_name}: '{to_compare_user_flag_value}' |  '{to_compare_dump_flag_value}'")
            comparison_table += f"<tr><td>{to_compare_user_flag_name}</td><td>{to_compare_user_flag_value}</td><td>{to_compare_dump_flag_value}</td></tr>\n"
#comparison_table += "</tabel>"

in_file_name = "../../../docs/table_flags/flags.in.md"
out_file_name = "../../../docs/table_flags/flags.md"

with open(in_file_name, "r", encoding='utf-8') as in_file:
    template_text = in_file.read()
    output_text = re.sub("<!--TABLE3 BEGIN-->.*<!--TABLE3 END-->", comparison_table, template_text, flags=re.DOTALL|re.MULTILINE)

    with open(out_file_name, "w", encoding='utf-8') as out_file:
        out_file.write(output_text)
