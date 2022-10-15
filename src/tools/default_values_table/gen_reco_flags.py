import all_flags_dump

all_flags = all_flags_dump.eicrecon_flags

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

reco_prefixes = [pref.lower() for pref in reco_prefixes]
print(reco_prefixes)

reco_flags = [flag[0] for flag in all_flags if flag[0].lower().startswith(tuple(reco_prefixes))]

for flag in reco_flags:
    print(flag)


