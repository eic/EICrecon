from all_flags_dump import eicrecon_flags

all_flag_records = eicrecon_flags

reco_prefixes = [
    "B0TRK",
    "BEMC",
    "BTRK",
    "BVTX",
    "DRICH",
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

low_case_reco_prefixes = [r.lower() for r in reco_prefixes]

# Find all flags that starts with reco_prefixes
all_reco_flag_records = [r
                         for r in all_flag_records
                         if r[0].lower().startswith(tuple(low_case_reco_prefixes))
                         and 'LogLevel' not in r[0]
                         and 'InputTags' not in r[0]]

# Calculate the longest flag and value lengths
max_name_len = len(max([r[0] for r in all_reco_flag_records], key=len))
max_default_val_len = len(max([r[1] for r in all_reco_flag_records], key=len))

# This is where we will generate text
all_python_content = "# format [ (flag_name, default_val, description), ...]\neicrecon_reco_flags=[\n"

# Go over reco_prefixes
for reco_prefix in reco_prefixes:

    print(reco_prefix)

    all_python_content += f"    # {reco_prefix}\n    # --------------\n"

    # Find all flags that starts with this prefix (excluding loggings and tags)
    reco_flag_records = [r
                         for r in all_flag_records
                         if r[0].lower().startswith(reco_prefix.lower())
                         and 'LogLevel' not in r[0]
                         and 'InputTags' not in r[0]
                         and 'input_tags' not in r[0]
                         and 'verbose' not in r[0]]

    # Generate a line for each flag_record that we found
    for record in reco_flag_records:
        flag_name = "'{}',".format(record[0])
        all_python_content += "    ({flag_name:<{flag_align}} {val_name:<{val_align}} '{descr}'),\n".format(
                              flag_name=flag_name,
            flag_align=max_name_len + 3,
            val_name="'{}',".format(record[1]),
            val_align=max_default_val_len + 3,
            descr=record[2])
        print(f"    {record}")

# This is how our generated file will end
all_python_content += "]\n\n"

# Save the output file
with open("reco_flags_dump.py", "w") as text_file:
    text_file.write(all_python_content)
