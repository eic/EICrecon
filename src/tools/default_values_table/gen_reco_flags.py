import all_flags_dump

all_flag_records = all_flags_dump.eicrecon_flags

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

#
# def filter_flags(flags_records, prefixes):
#
#     low_case_prefixes = [p.lower() for p in prefixes]
#
#     for flag in flags:
#         if not flag.lower()


low_case_prefixes = [p.lower() for p in reco_prefixes]

print(reco_prefixes)

reco_flag_records = [r
                     for r in all_flag_records
                     if r[0].lower().startswith(tuple(low_case_prefixes))
                     and 'LogLevel' not in r[0]
                     and 'InputTags' not in r[0]]
max_name_len = len(max([r[0] for r in reco_flag_records], key=len))
max_default_val_len = len(max([r[1] for r in reco_flag_records], key=len))

all_python_content = "# format [ (flag_name, default_val, description), ...]\neicrecon_reco_flags=[\n"

for record in reco_flag_records:
    flag_name = "'{}',".format(record[0])
    all_python_content += "    ({flag_name:<{flag_align}} {val_name:<{val_align}} '{descr}'),\n".format(
                          flag_name=flag_name,
        flag_align=max_name_len + 3,
        val_name="'{}',".format(record[1]),
        val_align=max_default_val_len + 3,
        descr=record[2])
    print(record)

all_python_content += "]\n\n"

with open("reco_flags_dump.py", "w") as text_file:
    text_file.write(all_python_content)

print(all_python_content)
